/*
 * Every line of code is my tear
 * Every tear is you
 */
#include "pangu.h"
#include "notifications_public.h"

#include <fcitx-config/iniparser.h>
#include <fcitx-utils/i18n.h>
#include <fcitx-utils/standardpath.h>
#include <fcitx-utils/utf8.h>
#include <fcitx/addonfactory.h>
#include <fcitx/inputcontext.h>
#include <fcitx/inputmethodentry.h>
#include <fcitx/statusarea.h>
#include <fcitx/userinterfacemanager.h>
#include <fcntl.h>

using namespace fcitx;

Pangu::Pangu(Instance *instance) : instance_(instance) {
    instance_->userInterfaceManager().registerAction("pangu",
                                                     &toggleAction_);
    auto filterKey = [this](Event &event) {
        if (!enabled_) {
            return;
        }
        auto &keyEvent = static_cast<KeyEventBase &>(event);
        if (!inWhiteList(keyEvent.inputContext())) {
            return;
        }
        if (keyEvent.key().states() || keyEvent.isRelease()) {
            return;
        }
        
        auto key = static_cast<uint32_t>(keyEvent.key().sym());
        if (key < 32) {  // 控制字符
            lastChar_ = 0;  // 重置lastChar_
            return;
        }
        
        if (needSpace(key, lastChar_)) {
            keyEvent.inputContext()->commitString(" ");
        }
        
        lastChar_ = key;
    };

    eventHandlers_.emplace_back(instance->watchEvent(
        EventType::InputContextKeyEvent, EventWatcherPhase::Default,
        [this, filterKey](Event &event) {
            auto &keyEvent = static_cast<KeyEvent &>(event);
            if (keyEvent.isRelease()) {
                return;
            }
            if (!inWhiteList(keyEvent.inputContext())) {
                return;
            }

            if (keyEvent.key().checkKeyList(config_.hotkey.value())) {
                setEnabled(!enabled_, keyEvent.inputContext());
                if (notifications()) {
                    notifications()->call<INotifications::showTip>(
                        "fcitx-fullwidth-toggle", _("Full width character"),
                        enabled_ ? "fcitx-fullwidth-active"
                                 : "fcitx-fullwidth-inactive",
                        _("Full width Character"),
                        enabled_ ? _("Full width Character is enabled.")
                                 : _("Full width Character is disabled."),
                        1000);
                }
                keyEvent.filterAndAccept();
                return;
            }

            return filterKey(event);
        }));
    commitFilterConn_ = instance_->connect<Instance::CommitFilter>(
        [this](InputContext *inputContext, std::string &str) {
            if (!enabled_ || !inWhiteList(inputContext)) {
                return;
            }
            
            auto len = utf8::length(str);
            std::string result;
            const auto *ps = str.c_str();
            
            for (size_t i = 0; i < len; ++i) {
                uint32_t wc;
                char *nps;
                nps = fcitx_utf8_get_char(ps, &wc);
                
                if (needSpace(wc, lastChar_)) {
                    result.append(" ");
                }
                
                result.append(ps, nps - ps);
                
                lastChar_ = wc;
                ps = nps;
            }
            
            str = std::move(result);
        });

    reloadConfig();
}

void Pangu::reloadConfig() {
    readAsIni(config_, "conf/pangu.conf");
    toggleAction_.setHotkey(config_.hotkey.value());
}

bool Pangu::inWhiteList(InputContext *inputContext) const {
    return toggleAction_.isParent(&inputContext->statusArea());
}

class PanguModuleFactory : public AddonFactory {
    AddonInstance *create(AddonManager *manager) override {
        return new Pangu(manager->instance());
    }
};

FCITX_ADDON_FACTORY(PanguModuleFactory)

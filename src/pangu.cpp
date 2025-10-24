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
#include <fcitx/inputcontext.h>
#include <fcitx/inputmethodentry.h>
#include <fcitx/statusarea.h>
#include <fcitx/userinterfacemanager.h>
#include <fcntl.h>
#ifndef FCITX_GETTEXT_DOMAIN
#define FCITX_GETTEXT_DOMAIN "fcitx5-pangu"
#endif

#ifndef FCITX_INSTALL_LOCALEDIR
#define FCITX_INSTALL_LOCALEDIR "/usr/share/locale"
#endif

using namespace fcitx;

Pangu::Pangu(Instance *instance) : instance_(instance) {
    registerDomain(FCITX_GETTEXT_DOMAIN, FCITX_INSTALL_LOCALEDIR);

    instance_->userInterfaceManager().registerAction("pangu",
                                                     &toggleAction_);
    auto filterKey = [this](Event &event) {
        if (!enabled_) {
            return;
        }
        auto &keyEvent = static_cast<KeyEventBase &>(event);
        if (keyEvent.key().states() || keyEvent.isRelease()) {
            return;
        }
        
        auto key = static_cast<uint32_t>(keyEvent.key().sym());
        
        uint32_t lastChar = getLastChar(keyEvent.inputContext());
        
        if (needSpace(key, lastChar)) {
            keyEvent.inputContext()->commitString(" ");
        }
    };

    eventHandlers_.emplace_back(instance->watchEvent(
        EventType::InputContextKeyEvent, EventWatcherPhase::Default,
        [this, filterKey](Event &event) {
            auto &keyEvent = static_cast<KeyEvent &>(event);
            if (keyEvent.isRelease()) {
                return;
            }

            if (keyEvent.key().checkKeyList(config_.hotkey.value())) {
                setEnabled(!enabled_, keyEvent.inputContext());
                if (notifications()) {
                    notifications()->call<INotifications::showTip>(
                        "fcitx-pangu-toggle", _("Pangu"),
                        enabled_ ? "fcitx-pangu-active"
                                 : "fcitx-pangu-inactive",
                        _("Pangu"),
                        enabled_ ? _("Pangu is enabled.")
                                 : _("Pangu is disabled."),
                        1000);
                }
                keyEvent.filterAndAccept();
                return;
            }

            return filterKey(event);
        }));
    commitFilterConn_ = instance_->connect<Instance::CommitFilter>(
        [this](InputContext *inputContext, std::string &str) {
            if (!enabled_) {
                return;
            }
            
            uint32_t lastChar = getLastChar(inputContext);
            uint32_t nextChar = getNextChar(inputContext);
            
            auto len = utf8::length(str);
            std::string result;
            const auto *ps = str.c_str();
            
            uint32_t firstChar;
            char *nps = fcitx_utf8_get_char(ps, &firstChar);
            if (needSpace(firstChar, lastChar)) {
                result.append(" ");
            }
            result.append(ps, nps - ps);
            lastChar = firstChar;
            ps = nps;
            
            for (size_t i = 1; i < len; ++i) {
                uint32_t wc;
                nps = fcitx_utf8_get_char(ps, &wc);
                
                if (needSpace(wc, lastChar)) {
                    result.append(" ");
                }
                
                result.append(ps, nps - ps);
                lastChar = wc;
                ps = nps;
            }

            if (len > 0 && needSpace(nextChar, lastChar)) {
                result.append(" ");
            }
            
            str = std::move(result);
        });

    reloadConfig();
}

FCITX_ADDON_FACTORY(PanguModuleFactory)

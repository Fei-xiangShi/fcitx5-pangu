/*
 * Every line of code is my tear
 * Every tear is you
 */
#ifndef _FCITX5_PANGU_PANGU_H_
#define _FCITX5_PANGU_PANGU_H_

#include <fcitx-config/configuration.h>
#include <fcitx-config/iniparser.h>
#include <fcitx-utils/i18n.h>
#include <fcitx/action.h>
#include <fcitx/addoninstance.h>
#include <fcitx/addonmanager.h>
#include <fcitx/instance.h>

FCITX_CONFIGURATION(PanguConfig, fcitx::Option<fcitx::KeyList> hotkey{
                                         this, "Hotkey", _("Toggle key")};)

class ToggleAction;

class Pangu final : public fcitx::AddonInstance {
    class ToggleAction : public fcitx::Action {
    public:
        ToggleAction(Pangu *parent) : parent_(parent) {}

        std::string shortText(fcitx::InputContext *) const override {
            return parent_->enabled_ ? _("Pangu enabled")
                                     : _("Pangu disabled");
        }
        std::string icon(fcitx::InputContext *) const override {
            return parent_->enabled_ ? "fcitx-pangu-active"
                                     : "fcitx-pangu-inactive";
        }

        void activate(fcitx::InputContext *ic) override {
            return parent_->setEnabled(!parent_->enabled_, ic);
        }

    private:
        Pangu *parent_;
    };

public:
    Pangu(fcitx::Instance *instance);

    void reloadConfig() override;
    const fcitx::Configuration *getConfig() const override { return &config_; }
    void setConfig(const fcitx::RawConfig &config) override {
        config_.load(config, true);
        fcitx::safeSaveAsIni(config_, "conf/pangu.conf");
        toggleAction_.setHotkey(config_.hotkey.value());
    }

    FCITX_ADDON_DEPENDENCY_LOADER(notifications, instance_->addonManager());

    bool inWhiteList(fcitx::InputContext *inputContext) const;

    void setEnabled(bool enabled, fcitx::InputContext *ic) {
        if (enabled != enabled_) {
            enabled_ = enabled;
            toggleAction_.update(ic);
        }
    }

private:
    bool enabled_ = false;
    fcitx::Instance *instance_;
    PanguConfig config_;
    std::vector<std::unique_ptr<fcitx::HandlerTableEntry<fcitx::EventHandler>>>
        eventHandlers_;
    fcitx::ScopedConnection commitFilterConn_;
    std::unordered_set<std::string> whiteList_;
    ToggleAction toggleAction_{this};
    uint32_t lastChar_ = 0;

    bool isAscii(uint32_t ch) const {
        return ch < 128;
    }

    bool isCJK(uint32_t ch) const {
        return (ch >= 0x4E00 && ch <= 0x9FFF) ||  // CJK统一汉字
               (ch >= 0x3040 && ch <= 0x309F) ||  // 平假名
               (ch >= 0x30A0 && ch <= 0x30FF);    // 片假名
    }

    bool needSpace(uint32_t current, uint32_t last) const {
        if (last == 0) return false;
        return (isAscii(last) && !isAscii(current)) ||
               (!isAscii(last) && isAscii(current));
    }
};

#endif // _FCITX5_PANGU_PANGU_H_

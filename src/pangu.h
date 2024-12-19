/*
 * Every line of code is my tear
 * Every tear is you
 */
#ifndef _FCITX5_PANGU_PANGU_H_
#define _FCITX5_PANGU_PANGU_H_

#include <fcitx-config/iniparser.h>
#include <fcitx-utils/i18n.h>
#include <fcitx-utils/utf8.h>
#include <fcitx/action.h>
#include <fcitx/addonfactory.h>
#include <fcitx/addoninstance.h>
#include <fcitx/addonmanager.h>
#include <fcitx/instance.h>

namespace fcitx {

FCITX_CONFIGURATION(PanguConfig,
                    Option<KeyList> hotkey{
                        this, "Hotkey", _("Toggle Key"), {Key("Control+Shift+space")}};
                    Option<bool> trailingSemiComma{
                        this, "No space before the trailing comma",
                        _("No space before the trailing comma"), true};
                    Option<bool> semiBracket{
                        this, "No spaces between brackets",
                        _("No spaces between brackets"), true};
                    Option<bool> semiQuote{
                        this, "No spaces between quotes",
                        _("No spaces between quotes"), true};
                    Option<bool> colon{
                        this, "No space before colon and semicolon",
                        _("No space before colon and semicolon"), true};
                    Option<bool> endSymbol{
                        this, "No space before end symbol",
                        _("No space before end symbol"), true};
                    )

class ToggleAction;

class Pangu final : public AddonInstance {
    class ToggleAction : public Action {
    public:
        ToggleAction(Pangu *parent) : parent_(parent) {}

        std::string shortText(InputContext *) const override {
            return parent_->enabled_ ? _("Pangu enabled")
                                     : _("Pangu disabled");
        }
        std::string icon(InputContext *) const override {
            return parent_->enabled_ ? "fcitx-pangu-active"
                                     : "fcitx-pangu-inactive";
        }

        void activate(InputContext *ic) override {
            return parent_->setEnabled(!parent_->enabled_, ic);
        }

    private:
        Pangu *parent_;
    };

public:
    Pangu(Instance *instance);
    const Configuration *getConfig() const override { return &config_; }
    FCITX_ADDON_DEPENDENCY_LOADER(notifications, instance_->addonManager());

    void reloadConfig() override {
        readAsIni(config_, "conf/pangu.conf");
        toggleAction_.setHotkey(config_.hotkey.value());
    }
    
    void setConfig(const RawConfig &config) override {
        config_.load(config, true);
        safeSaveAsIni(config_, "conf/pangu.conf");
        toggleAction_.setHotkey(config_.hotkey.value());
    }

    void setEnabled(bool enabled, InputContext *ic) {
        if (enabled != enabled_) {
            enabled_ = enabled;
            toggleAction_.update(ic);
        }
    }

private:
    bool enabled_ = true;
    Instance *instance_;
    PanguConfig config_;
    std::vector<std::unique_ptr<HandlerTableEntry<EventHandler>>>
        eventHandlers_;
    ScopedConnection commitFilterConn_;
    ToggleAction toggleAction_{this};
    uint32_t lastChar_ = 0;

    bool isAscii(uint32_t ch) const {
        return ch < 128 && ch != 32;
    }

    bool isCJK(uint32_t ch) const {
        return (ch >= 0x4E00 && ch <= 0x9FFF) ||  // CJK统一汉字
               (ch >= 0x3040 && ch <= 0x309F) ||  // 平假名
               (ch >= 0x30A0 && ch <= 0x30FF);    // 片假名
    }

    bool needSpace(uint32_t current, uint32_t last) const {
        if (last == 0 || current == 0) {
            return false;
        }
        
        bool currentIsAscii = isAscii(current);
        bool currentIsCJK = isCJK(current);
        bool lastIsAscii = isAscii(last);
        bool lastIsCJK = isCJK(last);
        
        bool needSpace = (lastIsAscii && currentIsCJK) ||
                        (lastIsCJK && currentIsAscii);
        
        if (!needSpace) {
            return false;
        }

        if (*config_.trailingSemiComma) {
            if (current == ',' || current == ';') {
                return false;
            }
        }

        if (*config_.semiBracket) {
            if ((last == '(' && isCJK(current)) ||
                (last == '[' && isCJK(current)) ||
                (last == '{' && isCJK(current)) ||
                (current == ')' && isCJK(last)) ||
                (current == ']' && isCJK(last)) ||
                (current == '}' && isCJK(last))) {
                return false;
            }
        }

        if (*config_.semiQuote) {
            if ((last == '"' && isCJK(current)) ||
                current == '"' && isCJK(last) ||
                last == '\'' && isCJK(current) ||
                current == '\'' && isCJK(last)) {
                return false;
            }
        }

        if (*config_.colon) {
            if (current == ':' || current == ';') {
                return false;
            }
        }

        if (*config_.endSymbol) {
            if (current == '.' || current == '?' || current == '!') {
                return false;
            }
        }

        return needSpace;
    }

    uint32_t getLastChar(InputContext *ic) const {
        if (!ic->capabilityFlags().test(CapabilityFlag::SurroundingText)) {
            return 0;
        }
        if (!ic->surroundingText().isValid()) {
            return 0;
        }

        const auto &text = ic->surroundingText().text();
        auto cursor = ic->surroundingText().cursor();
        if (cursor <= 0) {
            return 0;
        }

        uint32_t lastChar;
        auto start = utf8::nextNChar(text.begin(), cursor - 1);
        utf8::getNextChar(start, text.end(), &lastChar);
        return lastChar;
    }

    uint32_t getNextChar(InputContext *ic) const {
        if (!ic->capabilityFlags().test(CapabilityFlag::SurroundingText)) {
            return 0;
        }
        if (!ic->surroundingText().isValid()) {
            return 0;
        }

        const auto &text = ic->surroundingText().text();
        auto cursor = ic->surroundingText().cursor();
        if (cursor >= text.size()) {
            return 0;
        }

        uint32_t nextChar;
        auto start = utf8::nextNChar(text.begin(), cursor);
        utf8::getNextChar(start, text.end(), &nextChar);
        
        return nextChar;
    }
};

class PanguModuleFactory : public AddonFactory {
    AddonInstance *create(AddonManager *manager) override {
        return new Pangu(manager->instance());
    }
};

}

#endif // _FCITX5_PANGU_PANGU_H_

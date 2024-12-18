# Fcitx5-Pangu

[![License: GPL v2](https://img.shields.io/badge/License-GPL%20v2-blue.svg)](https://www.gnu.org/licenses/old-licenses/gpl-2.0.en.html)

Fcitx5-Pangu 是一个 [Fcitx5](https://github.com/fcitx/fcitx5) 的扩展模块，它可以在中日韩文字与英文字母之间自动插入空格，让文字的排版更加美观。

## ✨ 功能特点

- 在 CJK 与 ASCII 字符之间自动插入空格
- 支持自定义快捷键开关
- 可配置的空格规则:
  - 是否在尾随半角逗号前添加空格
  - 是否在半角括号之间添加空格
  - 是否在半角引号之间添加空格
  - 是否在半角冒号和分号前添加空格
  - 是否在半角句号、问号、感叹号前添加空格
- 支持多语言界面 (简体中文、繁体中文、日语、韩语、俄语)

## 📦 安装

### 从源码编译

需要的依赖:

- cmake >= 3.10
- extra-cmake-modules
- fcitx5
- gettext

```bash
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=Release ..
make
sudo make install
```

### Arch Linux

可以从 AUR 安装:

```bash
yay -S fcitx5-pangu
```

## ⚙️ 配置

1. 重启 Fcitx5 或重新加载配置
2. 在 Fcitx5 配置工具中找到 "Pangu" 模块
3. 可以设置:
   - 开关快捷键 (默认为 Ctrl+Shift+Space)
   - 自定义空格插入规则

## 🌏 本地化

目前支持以下语言:

- 简体中文
- 繁体中文
- 日语
- 韩语
- 俄语

欢迎贡献更多语言的翻译！

## 📝 许可证

本项目采用 [GPL-2.0-or-later](LICENSES/GPL-2.0-or-later.txt) 许可证。

## 🙏 致谢

- [Fcitx5](https://github.com/fcitx/fcitx5) - 下一代输入法框架
- [Pangu.js](https://github.com/vinta/pangu.js) - 为排版强迫者造福的 JavaScript 库

## 🤝 贡献

欢迎提交 Issue 和 Pull Request！

---

> 打字的時候不喜歡在中文和英文之間加空格的人，感情路都走得很辛苦，有七成的比例會在 34 歲的時候跟自己不愛的人結婚，而其餘三成的人最後只能把遺產留給自己的貓。畢竟愛情跟書寫都需要適時地留白。

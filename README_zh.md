# EMS (嵌入式音乐脚本) 解析器

[English Version](README.md) | 中文版本

一个专为嵌入式系统和单音乐器设计的轻量级音乐记谱格式。

## 特性

- 简单的字符串记谱法：`(120)1,2,3,4,5,6,7,1^`
- 最小内存占用
- 便于微控制器解析
- 人类可读格式
- 可扩展语法

## 格式

- 参见 [格式说明](doc/pages/format_zh.md)

## 示例

```c
play_music_string("(120)1,2,3,4,5,6,7,1^");
// BPM=120，四分音符，C大调音阶
```

## 待办事项

- [ ] 格式与文档
- [ ] 从MIDI转换
- [ ] C语言解析器

## 致谢

- EMS的灵感来源于由 `Celeca` 创建的 [simai](https://w.atwiki.jp/simai/pages/1003.html)。

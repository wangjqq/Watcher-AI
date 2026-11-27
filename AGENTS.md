# Watcher-AI 开发约定

## 项目入口

- `firmware/` 是 ESP32-S3 的 ESP-IDF 固件，包含设备 UI、配网、REST API 和配置持久化。硬件接线以 [PINMAP](firmware/PINMAP.md) 为准。
- `web/` 是 React / TypeScript 管理界面。Vite 构建产物写入 `firmware/web_dist/`，随后嵌入固件。构建步骤见 [README](README.md#构建固件)。
- `docs/hardware/` 包含硬件资料和厂商示例。定位产品运行逻辑时先看 `firmware/main/` 和 `web/src/`。
- 当前功能和使用方式见 [README](README.md#使用流程)。[Trae 项目规则](.trae/rules/project_rules.md) 补充产品背景和早期设计。

## 定位与修改

- 仓库根目录存在 `.codegraph/` 时，优先通过 `codegraph_explore` 定位代码和调用链，并在查询中指定产品目录或相关符号。没有索引时使用常规搜索。不要使用 GitNexus。
- 修改 API 或持久化配置时，核对 `firmware/main/network/http_server.c`、`firmware/main/common/app_config.c`、`web/src/api/client.ts` 和 `web/src/types.ts` 中受影响的契约。涉及配置结构时检查已有 NVS 数据读取和默认值。改动布局坐标或尺寸时，同时检查 `web/src/constants.ts` 与 `firmware/main/display/display.h`。
- 烧录固件、写入字库分区、执行 OTA 或重置设备前，须取得用户授权并确认目标设备。

## 按改动验证

| 改动范围 | 验证方式 |
| --- | --- |
| 文档或规范 | 核对链接、命令与现行代码，运行 `git diff --check`。 |
| Web 页面或类型 | 检查受影响的交互与数据流。`web/` 下的 `npm run typecheck` 用于类型验证，`npm run build` 用于验证固件所需的网页产物，按改动范围与耗时选择。 |
| API 或持久化配置 | 核对请求、响应和默认值，涉及已有配置时验证读取兼容性。具备设备环境时检查保存及重启后读取。 |
| 固件与设备交互 | 核对相关源码和配置。需要固件构建时在 `firmware/` 运行 `idf.py build`。显示、触摸、配网和 OTA 的实际行为需要在目标设备上验证。 |

默认运行与改动直接相关的快速检查。预计耗时较长的构建或全量检查按用户要求执行。交付时列出已执行的检查，并说明仍需真机或其他环境确认的行为。

## 提交与推送

- 每次完成一项代码、文档或配置改动并执行必要检查后，在结束任务前提交该改动，并推送到当前工作分支对应的远端分支。当前分支没有上游时，推送时设置上游。
- 提交前核对工作区和暂存区，只纳入本次任务的改动。提交信息应说明改动目的。推送后确认结果，并在交付时报告提交号和远端分支。提交或推送失败时说明具体原因及当前状态。

## 用户文档

- 用户可见功能、操作步骤或入口变化时，更新受影响的网页手册 [UserManual.tsx](web/src/pages/UserManual.tsx) 和设备手册 [app_ui.c 中的 `s_manual_pages`](firmware/main/display/app_ui.c)。项目概览、构建、部署或主要使用流程变化时同步更新 [README](README.md)。纯内部重构按需更新开发说明。
- 当前运行时手册由上述两个入口维护。`docs/manual.json` 和 `firmware/tools/gen_manual.py` 尚未接入这两个入口。

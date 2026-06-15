# HEart 使用教程

这份教程面向一个很具体的场景：你想马上用 HEart 编一个新的 CKKS/FHE 测试代码。

## 1. 调用格式

推荐显式调用 skill：

```text
$HEart target=OpenFHE project=<project-slug> ...
```

可选目标：

- `target=OpenFHE`：OpenFHE 1.5.1 / CPU / C++
- `target=Lattigo`：Lattigo 6.2.0 / CPU / Go
- `target=FlyHE`：Phantom/FlyHE / GPU
- `target=flyfhe`：同 `target=FlyHE`

`project=<project-slug>` 是项目名。建议用短 ASCII 名字，例如 `smoke-logreg`、`gpu-matmul-test`。

## 2. 最小测试提示词

如果你只是想快速试一下，用这个：

```text
$HEart target=OpenFHE project=smoke-logreg
帮我写一个加密向量逻辑回归推理的测试代码。
先输出 Design Note，等我确认后再写代码。
```

HEart 正常应该先给你 Design Note，而不是直接写代码。

## 3. OpenFHE 测试模板

```text
$HEart target=OpenFHE project=test-logreg-openfhe
请设计一个 8 维加密向量逻辑回归推理测试。
要求：
1. 使用 CKKS
2. x 是密文向量，w 和 b 是明文参数
3. 做 dot product 后接 sigmoid 多项式近似
4. 先给 Design Note
5. 我确认后再写 C++ 代码
6. 代码必须打印每个关键密文的 level/scale
7. 必须包含明文 reference 和逐 slot max abs / rel error
```

适合测试：

- OpenFHE routing 是否生效
- Design Note 是否先出现
- 代码是否包含明文 reference
- scale/level 是否被显式跟踪

## 4. Lattigo 测试模板

```text
$HEart target=Lattigo project=test-logreg-lattigo
请设计一个 CKKS 逻辑回归推理测试，目标是 Go 代码。
先给 Design Note，我确认后再写代码。
要求显式安排 Rescale、DropLevel、rotation keys 和 relin keys。
```

适合测试：

- Lattigo 显式 scale 管理
- `Rescale` / `DropLevel` 区分
- Go 代码结构

## 5. FlyHE / GPU 测试模板

```text
$HEart target=FlyHE project=test-gpu-matmul
请设计一个大批量 CKKS 矩阵乘测试，目标是 FlyHE GPU。
先给 Design Note，我确认后再写代码或运行笔记。
必须包含显存预算、host-device transfer 计划、rotation/key 计划和验证方法。
```

适合测试：

- GPU routing 是否生效
- 是否加载 FlyHE/GPU 特有注意事项
- 是否讨论显存、传输、stream/sync

## 6. 项目记忆模块

当你给出 `project=<slug>` 并开始一个具体项目时，HEart 应该在项目根目录维护：

```text
.heart-memory/
  project.md
  rules.md
  sessions/YYYY-MM-DD.md
  decisions.md
  artifacts.md
  open-questions.md
```

如果需要手动初始化，可以运行：

```powershell
HEart/scripts/init-heart-memory.ps1 `
  -ProjectRoot <your-project-root> `
  -ProjectSlug smoke-logreg `
  -TargetLibrary OpenFHE `
  -ProjectSummary "CKKS logistic regression smoke test"
```

这些文件记录：

- `project.md`：项目基本信息、目标库、输入输出、动态范围假设
- `rules.md`：这个项目自己的规则
- `sessions/`：每次会话日志
- `decisions.md`：设计决策
- `artifacts.md`：生成或修改的文件
- `open-questions.md`：仍需确认的问题

## 7. 你应该检查什么

让 HEart 生成代码后，至少检查：

- 是否先给了 Design Note
- 是否列出输入/输出 shape 和动态范围
- 是否列出乘法深度和 rescale schedule
- 是否列出 rotation steps 和 key plan
- 是否说明参数满足 128-bit security，或标 `[需人工确认]`
- 是否有明文 reference
- 是否报告 max abs / rel error
- 是否没有在密文上分支

## 8. 常见用法

让 HEart 只做设计：

```text
$HEart target=OpenFHE project=my-test
只给 Design Note，不写代码。
```

让 HEart 审查代码：

```text
$HEart target=OpenFHE project=my-test
请 review 这个 CKKS 代码，重点检查 scale/level、relin/rescale、rotation keys 和明文误差验证。
```

让 HEart debug：

```text
$HEart target=Lattigo project=my-debug
我的 CKKS 结果误差突然变大，请按 gotcha 目录帮我定位可能原因。
```

## 9. 推荐第一步

第一次测试建议从 OpenFHE smoke test 开始：

```text
$HEart target=OpenFHE project=smoke-logreg
帮我写一个加密向量逻辑回归推理的测试代码。
先输出 Design Note，等我确认后再写代码。
```

这个测试足够小，但会覆盖 HEart 最关键的工作流：target routing、Design Note、scale/level、keys、plaintext reference 和 slotwise error。

---
# HEart Skill 构建 — Agent 标准规则
## 项目
- 这是名为 HEart(v1.0)的 skill 构建工作区,根目录为当前目录,可交付 skill 在 ./HEart/。
- 三个【只读,禁止修改】源码目录:
  - ./openfhe-development-1.5.1 (OpenFHE 1.5.1, CPU)
  - ./lattigo-6.2.0 (Lattigo 6.2.0, CPU)
  - ./FlyHE-main (GPU 库, 基于 Phantom)
- 论文在 ./papers,蒸馏知识在 ./HEart/references。
## 每步必做
- 产物写到指定文件,不要只在对话里输出。
- 完成后在 ./PROGRESS.md 末尾追加:日期、完成步骤、产出文件清单、遗留/不确定点。
- 不确定的事实标 [需人工确认],禁止编造。
## 蒸馏哲学(关键)
- 蒸馏【可操作、面向决策】的知识,不是论文综述。每条尽量落到:不变量 / 代价(对 scale、level/深度、noise、时间、内存、所需 key 的影响)/ 陷阱(症状→成因→修法)/ 惯用法 / 决策规则。
- 每条标注来源(论文 slug 或源码文件路径)。文件保持聚焦简洁。
- SKILL.md 必须精简;细节一律放 references/ 并用相对链接引用。
## 版本钉死
- OpenFHE 1.5.1 / Lattigo 6.2.0 / GPU 库以其 README 版本为准;库相关写法都注明对应版本。
## 文档语言
- references 用英文(与论文/源码一致),每篇开头可加一段中文摘要。
## FHE 设计纪律(写任何 CKKS 代码前先产出"设计笔记")
1) 重述 f:输入/输出形状与动态范围。
2) 写成算术电路:乘法深度、总乘法数;非线性→选逼近多项式及阶数/深度。
3) 打包布局:输入到 slot 的映射;枚举所有 rotation 步数。
4) 选参数:N/素数链/Δ/slots 满足 深度+精度+128bit 安全;判断是否 bootstrap 及位置。
5) 列出要生成的 key:relin、Galois 集合、bootstrap。
6) 实现:每个密文显式跟踪 level 与 scale(注释/断言);绝不在密文上分支。
7) 验证:写明文 reference,逐 slot 比对,报告 max abs/rel error。
## v1.1 扩展约定(现在留口子)
- 二创/项目知识放 ./HEart/references/projects/<project-slug>.md,必须引用 core、禁止改 core。
- SKILL.md 的 "Project Registry" 区块 append-only:加项目=加一行,不动其它。
---

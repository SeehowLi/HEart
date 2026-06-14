# HEart Build Progress

## 2026-06-13 - 阶段0完成

完成步骤:
- 创建 HEart skill 工作区骨架。
- 写入根目录 Agent 标准规则。
- 写入构建阶段清单 0~6。

产出文件清单:
- AGENTS.md
- BUILD_PLAN.md
- PROGRESS.md
- HEart/SKILL.md
- HEart/references/core/
- HEart/references/libs/
- HEart/references/projects/
- HEart/scripts/
- papers/pdf/
- papers/text/

遗留/不确定点:
- 无。

## 2026-06-13 - 论文引用索引完成

完成步骤:
- 扫描 `openfhe-development-1.5.1`、`lattigo-6.2.0`、`FlyHE-main` 中的 README、docs、CITATION/BibTeX、参考文献章节和关键源码注释。
- 去重整理三库引用的学术论文，标注引用库与类别。
- 单独标记 CKKS 核心理论、RNS-CKKS、bootstrapping、key-switching/hybrid、非线性逼近/comparison、GPU/NTT 等类别。
- 生成免费下载 URL 列表，优先使用 ePrint/arXiv PDF 链接。

产出文件清单:
- papers/PAPER_INDEX.md
- papers/download_list.txt

遗留/不确定点:
- 部分条目在本地源码/文档中只给出 ePrint/arXiv 编号或作者简称，标题、作者、venue 已按规则标 `[需人工确认]`。
- 部分 IEEE/Springer/USENIX 条目未找到本地明确的免费 PDF 直链，索引中保留原引用或标 `[需人工确认]`。
- 未联网逐条验证 PDF 可下载性；`download_list.txt` 已按 ePrint/arXiv 的常规 PDF URL 形式生成。

## 2026-06-13 - PDF 下载与文本抽取完成

完成步骤:
- 读取 `papers/download_list.txt` 并逐条尝试用 `curl.exe` 下载 PDF 到 `papers/pdf/`。
- 使用简短 ASCII slug 命名成功下载的 PDF。
- 对成功下载的 PDF 使用 `pdftotext` 转成纯文本到 `papers/text/`。
- 生成 `papers/DOWNLOAD_REPORT.md`，汇总成功、失败与需人工处理的 URL。

产出文件清单:
- papers/DOWNLOAD_REPORT.md
- papers/pdf/idealhom-bv.pdf
- papers/pdf/arxiv-2009-00349.pdf
- papers/pdf/arxiv-2103-16400.pdf
- papers/pdf/arxiv-1205-2926.pdf
- papers/pdf/arxiv-2003-04510.pdf
- papers/pdf/arxiv-2012-01968.pdf
- papers/pdf/arxiv-1407-3383.pdf
- papers/pdf/arxiv-1303-6257.pdf
- papers/text/idealhom-bv.txt
- papers/text/arxiv-2009-00349.txt
- papers/text/arxiv-2103-16400.txt
- papers/text/arxiv-1205-2926.txt
- papers/text/arxiv-2003-04510.txt
- papers/text/arxiv-2012-01968.txt
- papers/text/arxiv-1407-3383.txt
- papers/text/arxiv-1303-6257.txt

遗留/不确定点:
- 共处理 85 个 URL；成功下载 8 个 PDF 并生成 8 个文本文件。
- 77 个 URL 需人工处理，主要原因是 ePrint 链接在当前环境下通过 `curl.exe` 返回 HTTP 403；详见 `papers/DOWNLOAD_REPORT.md`。
- 未对手动失败项改用浏览器交互下载。

## 2026-06-13 - 核心理论蒸馏完成

完成步骤:
- 重新核对 `papers/pdf/` 与 `papers/text/`，当前 PDF 数量 82、文本数量 82，按 basename 检查无缺失文本。
- 只读取 `papers/text/` 中的核心理论论文文本，覆盖 CKKS 原始论文、Full-RNS/RNS-CKKS、bootstrapping、hybrid key-switching 相关来源。
- 按 AGENTS.md 的蒸馏哲学写入 CKKS 心智模型与操作语义手册，强调 scale、level、noise、RNS/NTT、近似正确性和工程陷阱。

产出文件清单:
- HEart/references/core/01-mental-model.md
- HEart/references/core/02-op-semantics.md

遗留/不确定点:
- 只关注当前已经下载并转成文本的 PDF；未下载论文未纳入本轮蒸馏。
- `虚部泄露` 在文档中按 real-only CKKS 流水线的虚部污染/泄入风险处理；是否构成安全意义上的 leakage 标为 `[需人工确认]`。
- 非核心理论论文虽已存在于 `papers/text/`，但本轮未读取和蒸馏。

## 2026-06-13 - 核心理论 reference 增补完成

完成步骤:
- 增补 `01-mental-model.md` 的 `Ciphertext Structure` 小节，说明 `(c0,c1)`、degree-2 `(c0,c1,c2)`、extended ciphertext、relinearization 与 `Mul -> Relinearize -> Rescale` 的关系。
- 增补 `01-mental-model.md` 的 `Security Co-Design` 小节，加入 Li-Micciancio approximate-decryption key-recovery 风险、noise flooding 规则、虚部污染区分，以及固定 `N` 下 `Q`/`P*Q` 的安全硬上限。
- 增补 `02-op-semantics.md` 的全局代价排序、OpenFHE/SEAL 命名陷阱、乘法前 level 对齐、lazy relinearization、`EvalSquare`、bootstrap rotation-key/BSGS 规则。
- 核对 `PAPER_INDEX.md` 后把 Add/Mul/Rotate 的主源调整为 `2016-421`/`2018-931`，`2020-1203` 作为操作语义次源保留。
- 确认并写明普通 CKKS 中 ciphertext-ciphertext 与 plaintext-ciphertext 乘法都会放大 scale，正常 scale schedule 下都需要后续 rescale。

产出文件清单:
- HEart/references/core/01-mental-model.md
- HEart/references/core/02-op-semantics.md
- PROGRESS.md

遗留/不确定点:
- Li-Micciancio 的 ePrint ID `2020/1533` 已由本地 `papers/text/2020-1533.txt` 与 `PAPER_INDEX.md` 核对；EUROCRYPT 2021 venue 仍沿用索引中的 `[需人工确认]`。
- HE standard / lattice estimator 的精确引用仍标为 `[需人工确认 HE-standard/lattice-estimator citation]`，后续应在 `05-security-parameters.md` 中蒸馏。
- `lib-*.md` 尚未创建，OpenFHE/SEAL 命名映射目前写在 `02-op-semantics.md` 并引用本地 OpenFHE 源码与官方 SEAL 源码路径。

## 2026-06-13 - 核心规则/配方/参数 playbook 蒸馏完成

完成步骤:
- 继续读取 `papers/text/` 中已下载并转成文本的理论论文、非线性逼近/comparison 论文与 CKKS bootstrapping 论文。
- 写入失败模式目录，覆盖 scale mismatch、level mismatch、深度耗尽、缺 rotation key、NTT 域不匹配、虚部污染、精度损失、参数/安全违例、CKKS approximate-decryption/IND-CPAD 泄露与 noise flooding 缓解。
- 写入算法配方 cookbook，覆盖 rotate-and-sum、inner product、diagonal/BSGS matrix-vector、Chebyshev/Paterson-Stockmeyer polynomial evaluation、sign/comparison composite minimax、ML nonlinearities、inverse/inverse-sqrt iteration skeleton、bootstrap substeps 与 placement。
- 写入参数选择 playbook，整理 `N`、slots、`Delta`、RNS 素数链、special primes `P`、固定 `N` 下 `log(Q*P)` 安全上限、bootstrap 判断与 128-bit lattice-estimator 校验流程。
- 对 `04-pattern-cookbook.md` 做补强，统一每个 recipe 的字段，并将 inverse-square-root 细节保留为 `[需人工确认]`，避免把未核实库/论文实现写成确定规则。

产出文件清单:
- HEart/references/core/03-invariants-gotchas.md
- HEart/references/core/04-pattern-cookbook.md
- HEart/references/core/05-params-playbook.md
- PROGRESS.md

遗留/不确定点:
- 本轮只使用 `papers/text/` 中已经存在的文本；未下载或未转文本的论文未纳入蒸馏。
- inverse-square-root 的具体 Newton/Goldschmidt 参数化与库实现仍标为 `[需人工确认 inverse-sqrt recipe]`。
- lattice-estimator 的精确命令行仍标为 `[需人工确认 exact estimator command]`，需后续结合项目环境或库脚本确认。

## 2026-06-13 - OpenFHE CKKS 库蒸馏完成

完成步骤:
- 检查 `HEart/references/core/04-pattern-cookbook.md`，确认每个配方都包含 `Depth cost` 与 `Required keys` 字段。
- 对 OpenFHE 1.5.1 只读源码目录做 CKKS 地图，定位 `src/pke/lib/scheme/ckksrns/`、RNS/base 层、key-switch 层与 `src/pke/examples/` 中 CKKS 相关 examples。
- 阅读核心算子实现，覆盖 `EvalMult`/relinearize、`Rescale`/`ModReduce`、`LevelReduce`、HYBRID/BV key switching、`KeyGen`/evaluation keygen、CKKS bootstrap setup/keygen/evaluation，以及 `FIXEDMANUAL`、`FIXEDAUTO`、`FLEXIBLEAUTO`、`FLEXIBLEAUTOEXT` scaling modes。
- 精读代表性 examples：`simple-real-numbers.cpp`、`rotation.cpp`、`simple-ckks-bootstrapping.cpp`，并补充 `polynomial-evaluation.cpp` 作为 polynomial/nonlinear 模板。
- 写入 OpenFHE 库参考，包含版本钉死、setup boilerplate、core/02 操作到 OpenFHE API 的映射、scale 管理模型、key 生成方式、bootstrap 用法、本库 gotchas 与按 7 步设计纪律拆解的 example 走读。
- 核对 `openfhe.md` 的操作名与 `02-op-semantics.md` 的 primitive 对齐，并确认 scaling mode 段明确说明 AUTO 档正常写法下不需要手动到处插 `Rescale`，`FIXEDMANUAL` 才需要显式安排。

产出文件清单:
- HEart/references/libs/openfhe.md
- PROGRESS.md

遗留/不确定点:
- OpenFHE 1.5.1 中 CKKS conjugation 在 `FHECKKSRNS` 内部实现并用于 bootstrap；未找到常规 `CryptoContext` public wrapper，已在 `openfhe.md` 标为 `[需人工确认 public conjugation API]`。
- 本轮未编译或运行 OpenFHE examples；结论来自本地源码和 examples 静态阅读。

## 2026-06-13 - Lattigo CKKS 库蒸馏完成

完成步骤:
- 只读扫描 `lattigo-6.2.0`，确认本地 6.2.0 的 CKKS 包路径为 `schemes/ckks`，bootstrapping 包路径为 `circuits/ckks/bootstrapping`，Scale 类型为 `core/rlwe.Scale`。
- 完成 Lattigo 仓库地图，列出 CKKS scheme、RLWE 基础层、polynomial/lintrans/bootstrap circuits，以及 CKKS 相关 examples。
- 阅读核心算子实现策略，覆盖 `Evaluator` 的 Add/Mul/MulRelin/Rescale/DropLevel/Rotate/Conjugate、RLWE key switching/relinearization/Galois key lookup、KeyGen 与 bootstrapping `GenEvaluationKeys`/`Bootstrap`。
- 精读 `templates/ckks`、`tutorials/ckks`、`ckks_sigmoid_minimax`、`ckks_bootstrapping/basics` examples，并按 7 步设计纪律写入走读。
- 写入 Lattigo 6.2.0 reference，包含 boilerplate、操作名到 `02-op-semantics.md` 的映射、显式 scale 管理模型、key 生成、bootstrap 用法、本库 gotchas 和预算 checklist。
- 核查 `lattigo.md` 中操作名覆盖 Add/AddPlain/MulConst/Mul/Relinearize/Rescale/Rotate/Conjugate/DropLevel/Bootstrap，并明确 Lattigo 没有 OpenFHE AUTO scaling mode，乘法后仍需显式安排 `Rescale`。

产出文件清单:
- HEart/references/libs/lattigo.md
- PROGRESS.md

遗留/不确定点:
- 未编译或运行 Lattigo examples；结论来自本地源码与 examples 静态阅读。
- 用户提示中的 `he/hefloat` 与本地 6.2.0 源码不一致；已按本地源码写为 `schemes/ckks` / `circuits/ckks/bootstrapping`，并在 gotchas 中标明路径风险。

## 2026-06-14 - FlyHE/Phantom GPU 库蒸馏完成

完成步骤:
- 只读扫描 `FlyHE-main/FlyHE-main` 顶层 README、`RLWE/phantom/README.md`、CMake、Phantom RLWE core、FlyHE CKKS boot layer、conversion examples 与 LWE/cuTFHEpp 入口。
- 确认真实项目名为 FlyHE；本地 README 未给出语义版本号，已在文档标为 `[需人工确认 version/tag]`。
- 区分 Phantom core 的 BGV/BFV/CKKS GPU 支持且 "without bootstrapping"，以及 FlyHE 仓库额外 `RLWE/boot` 中的 CKKS bootstrapping examples/API。
- 梳理 GPU 实现策略: `cuda_auto_ptr`/`cudaMallocAsync` 显存管理、device-to-device async copy、2D radix-8 NTT/iNTT CUDA kernels、GPU key switching 的 mod-up/inner-product/mod-down、host-device encode/decode 边界、rotation/bootstrapping 同步点、hoisting 多 rotation 复用。
- 精读 `3_ckks.cu`、`4_kernel_fusing.cu`、`RLWE/boot/examples/bootstrapping.cu`、`conversion/example/extract.cu`、`conversion/example/repack.cu`，并按工程规则写入速查与 gotchas。
- 写入 GPU FHE 通用注意事项，重点覆盖显存预算、主机-设备传输、kernel 启动/同步、stream/batch 并行、NTT 状态和相比 CPU 库多出的工作流步骤。

产出文件清单:
- HEart/references/libs/phantom-flyhe.md
- HEart/references/core/gpu-considerations.md
- PROGRESS.md

遗留/不确定点:
- FlyHE 本地 README 未声明明确版本号或 release tag。
- 顶层 README 对修改版 Phantom 许可的描述与本地 `RLWE/phantom/README.md` 的 GPLv3 描述存在差异，已标为 `[需人工确认 license status]`。
- FlyHE README 未单独引用 NTT kernel 论文；NTT 技术结论主要来自本地 CUDA 实现与 Phantom/GPU 论文引用，具体 paper-to-code 映射仍需人工确认。
- 未编译或运行 FlyHE examples；结论来自本地源码和 examples 静态阅读。

## 2026-06-14 - 三库选型对比完成

完成步骤:
- 基于 `openfhe.md`、`lattigo.md`、`phantom-flyhe.md` 汇总 CPU vs GPU 的决策规则，覆盖数据规模、bootstrap、吞吐、部署环境、host-device transfer 与 GPU 研究代码风险。
- 汇总 CPU 内部 OpenFHE vs Lattigo 选型规则，突出 OpenFHE AUTO scale/功能广度与 Lattigo Go/显式控制差异。
- 写入特征矩阵，覆盖语言/绑定、scale 管理、bootstrap 有无、典型优势和典型坑。
- 添加场景 playbook 与选择 checklist，并交叉引用三份 libs 文档及 GPU considerations。

产出文件清单:
- HEart/references/libs/comparison.md
- PROGRESS.md

遗留/不确定点:
- OpenFHE 其它语言绑定未在本地 `openfhe.md` 中核实，矩阵中标为 `[需人工确认 bindings]`。
- FlyHE 版本号/license/NTT paper-to-code 映射仍沿用 `phantom-flyhe.md` 中的不确定点。

## 2026-06-14 - HEart SKILL v1.0 初稿完成

完成步骤:
- 写入 `HEart/SKILL.md` 的 YAML frontmatter，钉死 name/version/description 和触发场景。
- 按指定结构写入 Target Selection、Mandatory Workflow、Never Do、Reference Index、Project Registry。
- 将 AGENTS.md 的 7 步 CKKS 设计纪律完整纳入 Mandatory Workflow，并为 GPU 目标补充显存预算和 host-device 传输规划。
- 创建 `HEart/CHANGELOG.md`，记录 v1.0 初稿内容。

产出文件清单:
- HEart/SKILL.md
- HEart/CHANGELOG.md
- PROGRESS.md

遗留/不确定点:
- `Project Registry` 目前为空表，后续新增项目必须 append-only，加一行并把细节写到 `references/projects/<project-slug>.md`。

## 2026-06-14 - HEart SKILL v1.0 路由与自检规则增补完成

完成步骤:
- 核对 `FlyHE-main/FlyHE-main` README、`RLWE/phantom/README.md` 与 CMake，未找到明确 FlyHE/Phantom 语义版本号，因此在 `SKILL.md` 的 GPU 库版本处标为 `[需人工确认 version]`。
- 在 description Trigger 列表追加中文触发词: 全同态加密、同态加密、隐私计算、加密推理、CKKS 方案。
- 在 Target Selection 顶部加入 CPU/GPU 或具体库不明确时的歧义解决规则: 只问一个短问题；用户要默认时选 OpenFHE (CPU)；始终显式说明目标。
- 在 Mandatory Workflow 顶部加入代码前必须先给用户 Design Note 的总纲，并在参数步骤补充 OpenFHE/Lattigo/SEAL scale-management 与 rescale schedule 要求。
- 新增 `Self-Check Before Returning` 小节，覆盖 Never Do 复查、level/scale 对照、key 生成确认、明文 reference 与 slotwise error harness。
- Load references 选择保持全部加载 core 01-05，并补充分层说明: 01-03 必读、04 用于 circuit/nonlinear、05 用于参数/安全。
- 更新 `HEart/CHANGELOG.md` 记录本次 v1.0 规则增补。

产出文件清单:
- HEart/SKILL.md
- HEart/CHANGELOG.md
- PROGRESS.md

遗留/不确定点:
- FlyHE/Phantom 真实语义版本号仍需人工确认；本地 README/CMake 未声明。

## 2026-06-14 - HEart 一致性审计完成

完成步骤:
- 审计 `HEart/SKILL.md` 与 `HEart/references/` 的引用覆盖，确认 10 个 references 文件均被 `SKILL.md` 直接链接。
- 检查 `HEart/**/*.md` 内部相对 Markdown 链接，最终 68 个链接均可解析，断链 0 个。
- 核对 core/02 与三份 libs 文件的算子术语，补齐 OpenFHE API mapping 中的 `Conjugate` 行，并标注公共封装名 `[需人工确认]`。
- 修正 `SKILL.md` 中未经确认的 `Phantom/FlyHE(1.0.0)` 声明，改为 `Phantom/FlyHE [需人工确认 version]`，与 `phantom-flyhe.md`、`comparison.md` 对齐。
- 核对版本钉死、gotcha 的 Symptoms/Cause/Fix 结构、pattern 的 Depth cost/Required keys 字段，以及中英文 description 触发词覆盖。
- 创建 `HEart/VALIDATION.md` 记录审计结果，并更新 `HEart/CHANGELOG.md`。

产出文件清单:
- HEart/VALIDATION.md
- HEart/SKILL.md
- HEart/references/libs/openfhe.md
- HEart/CHANGELOG.md
- PROGRESS.md

遗留/不确定点:
- FlyHE/Phantom 真实语义版本号仍需人工确认；本地 README/CMake 未声明。
- OpenFHE 面向用户的首选 Conjugate 公共封装名仍需人工确认；本地源码确认了 `FHECKKSRNS::Conjugate`、`ConjugateKeyGen` 与 `EvalAutomorphism` 路径。
- 本次只做文档一致性与链接审计，未编译或运行三个库。

## 2026-06-14 - OpenFHE CKKS 逻辑回归加密推理示例

完成步骤:
- 按 HEart 设计纪律为 OpenFHE 1.5.1 / CPU / C++ 写出 CKKS 逻辑回归推理设计笔记: 函数形状、logit 范围、算术电路、packing/rotation、参数/scale-level 计划、key 集合。
- 新增项目知识 `HEart/references/projects/openfhe-logreg-inference.md`, 记录加密特征向量、明文权重/偏置、EvalSum 点积、EvalLogistic sigmoid 近似和 slotwise 验证规则。
- 新增可复用示例代码 `HEart/examples/openfhe_logreg_inference.cpp`, 包含 OpenFHE 1.5.1 API、level/scale 打印、明文 reference、max abs/rel/imag error 报告。
- 在 `HEart/SKILL.md` 的 Project Registry 中 append-only 添加 `openfhe-logreg-inference` 项目行。
- 静态核对 OpenFHE 源码示例 `function-evaluation.cpp` 与 `inner-product.cpp`, 确认 `EvalLogistic`、`EvalSumKeyGen`、`EvalSum` 使用方式与本示例一致。

产出文件清单:
- HEart/references/projects/openfhe-logreg-inference.md
- HEart/examples/openfhe_logreg_inference.cpp
- HEart/SKILL.md
- PROGRESS.md

遗留/不确定点:
- 真实特征维度、特征/权重归一化方式、logit 动态范围仍需按具体模型确认；当前示例假设 `z=<w,x>+b` 位于 `[-5,5]` [需人工确认]。
- 本地未发现已构建的 OpenFHE build/install 目录, 因此本次未实际编译或运行示例；已做 API 静态核对。

## 2026-06-14 - HEart 端到端自测完成

完成步骤:
- 启动两个全新子会话，仅传入 `HEart` skill 路径，分别测试中文逻辑回归推理触发与 FlyHE/GPU CKKS 矩阵乘路径。
- 保存测试1全过程到 `HEart/_selftest/test1/`: 初始中文请求、目标选择问题、默认 OpenFHE 路径继续后的输出、评分表。
- 保存测试2全过程到 `HEart/_selftest/test2/`: GPU/FlyHE 请求、设计笔记草案、VRAM/host-device transfer 证据、评分表。
- 汇总写入 `HEart/_selftest/REPORT.md`: 测试1整体失败、测试2通过，并列出 workflow/观测性/自测隔离缺口。
- 根据暴露缺口更新 `BUILD_PLAN.md` 的 `待补` 小节。

产出文件清单:
- HEart/_selftest/REPORT.md
- HEart/_selftest/test1/transcript.md
- HEart/_selftest/test1/score.md
- HEart/_selftest/test2/transcript.md
- HEart/_selftest/test2/score.md
- BUILD_PLAN.md
- PROGRESS.md

遗留/不确定点:
- 测试1的新会话在共享工作区生成了 `HEart/examples/openfhe_logreg_inference.cpp`、`HEart/references/projects/openfhe-logreg-inference.md`，并 append 了 `SKILL.md` Project Registry；是否保留为 v1.1 项目示例需人工确认。
- 测试1暴露 Mandatory Workflow 执行缺口: Design Note 写在项目文件中，但没有先作为用户可见回复展示再生成代码。
- 测试2无法从最终回复直接证明具体 reference 文件被加载，只能以输出行为作为证据。

## 2026-06-14 - 用户请求: 加密向量逻辑回归推理

完成步骤:
- 按 HEart 路由规则选择默认目标 `OpenFHE 1.5.1 / CPU / C++`，因为用户未指定 CPU/GPU 或库。
- 复核已安装 HEart skill 中的核心 CKKS 规则与 OpenFHE 1.5.1 API 映射，并静态核对本地 OpenFHE 1.5.1 源码中的 `EvalLogistic`、`EvalSumKeyGen`、`EvalSum`、`GetLevel`、`GetScalingFactor` 接口。
- 确认现有交付物已经覆盖加密特征向量、明文权重/偏置、CKKS `EvalSum` 点积、`EvalLogistic` sigmoid 近似、level/scale 打印、明文 reference 与 slotwise max abs/rel/imag error 报告。

产出文件清单:
- HEart/examples/openfhe_logreg_inference.cpp
- HEart/references/projects/openfhe-logreg-inference.md
- PROGRESS.md

遗留/不确定点:
- 真实模型的特征维度、归一化方式、权重范围和 logit 动态范围仍需按具体训练产物确认；当前示例假设 `z=<w,x>+b` 落在 `[-5,5]` [需人工确认]。
- 本地未发现已构建的 OpenFHE build/install 目录，因此本次未实际编译或运行示例；已完成源码 API 静态核对。
## 2026-06-14 - FlyHE batched CKKS matrix multiplication note

Completed steps:
- Loaded HEart core CKKS references, GPU/FlyHE references, and local FlyHE matrix multiplication sources without modifying read-only source directories.
- Identified the local FlyHE MatMul baseline as `RLWE/boot/examples/nn.cu` plus `RLWE/boot/src/matrix_mul.cu`, with shape `4096x768 * 768x64`.
- Wrote a project note covering the CKKS design note, packing/rotation keys, scale-level schedule, VRAM budget, host-device transfer plan, runbook, validation requirements, and scaling rules.
- Appended the project to `HEart/SKILL.md` Project Registry.
- Checked local GPU/toolchain availability: `nvidia-smi` sees an RTX 3070 Laptop GPU with 8GB VRAM, but `nvcc` is not in PATH, so build/run verification was not performed.

Output files:
- HEart/references/projects/flyhe-batched-ckks-matmul.md
- HEart/SKILL.md
- PROGRESS.md

Remaining / uncertain:
- FlyHE local checkout has no semantic version tag in README/CMake; exact version remains [need manual confirmation].
- Security for `N=8192`, `{60,40,60}`, and FlyHE key-switching parameters was not estimator-verified [need manual confirmation].
- The example divides decrypted MatMul output by `2.0` before comparing calibration; the reason must be confirmed before publication [need manual confirmation].
- Actual build/run requires a CUDA toolkit visible to CMake/nvcc; this PowerShell session did not have `nvcc` in PATH.

## 2026-06-14 - HEart end-to-end self-test rerun

Completed steps:
- Ran two fresh sub-agent sessions with only the `HEart` skill path attached, testing the Chinese logistic-regression trigger and the FlyHE/GPU CKKS matrix-multiplication path.
- Rewrote `HEart/_selftest/test1/transcript.md` and `HEart/_selftest/test1/score.md` with the fresh-session output and pass/fail evidence.
- Rewrote `HEart/_selftest/test2/transcript.md` and `HEart/_selftest/test2/score.md` with the FlyHE output, generated-project evidence, and workflow caveat.
- Rewrote `HEart/_selftest/REPORT.md` with a consolidated summary, exposed gaps, and file list.
- Updated `BUILD_PLAN.md` pending items for stricter target selection, stricter Design Note gating, self-test isolation, FlyHE project-registry cleanup, and future CUDA build/run validation.

Output files:
- HEart/_selftest/REPORT.md
- HEart/_selftest/test1/transcript.md
- HEart/_selftest/test1/score.md
- HEart/_selftest/test2/transcript.md
- HEart/_selftest/test2/score.md
- BUILD_PLAN.md
- PROGRESS.md

Remaining / uncertain:
- Test 1 failed the target-selection gate and Design Note-before-implementation gate.
- Test 2 passed the GPU/FlyHE design criteria but still wrote durable project files before returning; treat this as a workflow-isolation caveat.
- The fresh sessions appended `openfhe-logreg-inference` and `flyhe-batched-ckks-matmul` to the Project Registry; whether to keep them as v1.1 examples needs manual confirmation.
- FlyHE MatMul was not compiled or run because `nvcc` was not in PATH [need manual confirmation in a CUDA-capable environment].

## 2026-06-14 - HEart self-test failure remediation

Completed steps:
- Read `HEart/_selftest/REPORT.md` and identified the failing items: ambiguous target selection and Design Note-before-implementation gating in Test 1.
- Strengthened `HEart/SKILL.md` target routing: ambiguous CPU/GPU/library prompts must ask exactly one target-selection question and stop; agents must not silently choose OpenFHE by default unless the user explicitly asks for a default.
- Strengthened `HEart/SKILL.md` workflow gating: Steps 1-5 Design Note must be the next user-visible artifact before code or durable file creation, and implementation/file writes require user confirmation unless explicitly requested.
- Added `Self-Test Mode` guidance to keep generated artifacts under `HEart/_selftest/<test>/artifacts/` and to expose loaded HEart references during scoring-only runs.
- Reran the failed Test 1 scenario in a fresh sub-agent with only the HEart skill attached. The response asked one target question and stopped: `OpenFHE 1.5.1 C++ CPU`, `Lattigo 6.2.0 Go CPU`, or `FlyHE/Phantom GPU`.
- Updated Test 1 transcript/score and consolidated `HEart/_selftest/REPORT.md`; Test 1 is now `PASS after fix`.
- Replaced stale absolute Markdown links in `HEart/_selftest/test2/transcript.md` with code-form paths to avoid selftest link-check noise.
- Updated `HEart/CHANGELOG.md`.

Output files:
- HEart/SKILL.md
- HEart/CHANGELOG.md
- HEart/_selftest/REPORT.md
- HEart/_selftest/test1/transcript.md
- HEart/_selftest/test1/score.md
- HEart/_selftest/test2/transcript.md
- PROGRESS.md

Remaining / uncertain:
- Project Registry still contains test-derived projects (`openfhe-logreg-inference`, `flyhe-batched-ckks-matmul`); cleanup needs explicit approval because the registry is append-only.
- FlyHE runtime remains unverified until a CUDA/nvcc-capable environment is available.

## 2026-06-14 - v1.0 complete

完成步骤:
- 定稿 HEart v1.0，确认 `HEart/SKILL.md` frontmatter 为 `version: "1.0"`。
- 更新 `HEart/CHANGELOG.md`，新增 v1.0 Formal 条目并列出 core/libs/project extension/self-test/validation 文件。
- 创建 `HEart/ROADMAP.md`，规划 v1.1 纳入基于 OpenFHE 为主的二创/项目知识，并明确项目模块扩展约定。
- 创建 `HEart/references/projects/README.md`，规定 v1.1 新增项目模块流程: 新建 `references/projects/<project-slug>.md`、追加 `SKILL.md` Project Registry 一行、禁止修改 core/libs 现有内容。
- 创建根目录 `README.md`，说明 HEart v1.0 用途、目录结构、使用方式、强制 CKKS workflow、v1.0 内容和状态。
- 创建 `.gitignore`，排除本地只读源码库与论文输入目录。
- 初始化 git 仓库，提交 v1.0 release，创建并推送 `v1.0` tag 到 `https://github.com/SeehowLi/HEart.git`。

产出文件清单:
- README.md
- .gitignore
- HEart/CHANGELOG.md
- HEart/ROADMAP.md
- HEart/references/projects/README.md
- PROGRESS.md

遗留/不确定点:
- FlyHE/Phantom 语义版本号仍需人工确认；本地 README/CMake 未声明。
- Project Registry 中的 test-derived project rows 是否保留为 v1.1 示例，仍待人工决定。
- FlyHE runtime 未在 CUDA/nvcc 环境下编译运行验证。

## 2026-06-14 - README usage example added

完成步骤:
- 在根目录 `README.md` 的 `Using The Skill` 后新增 `Minimal Example`。
- 示例展示用户如何指定 HEart + OpenFHE CPU 目标，以及 HEart 应先输出 Design Note 并等待确认，再生成代码。
- 重新检查 Markdown 相对链接。
- 提交并推送 README 更新到 GitHub。

产出文件清单:
- README.md
- PROGRESS.md

遗留/不确定点:
- 本次未改动 `v1.0` tag；这是 v1.0 发布后的 README 文档补充提交。

## 2026-06-14 - HEart local install interface

完成步骤:
- 恢复并确认 `HEart/SKILL.md` frontmatter `version: "1.0"`。
- 在 `HEart/SKILL.md` description 与 Invocation Interface 中明确稳定调用接口: `$HEart target=OpenFHE`, `$HEart target=Lattigo`, `$HEart target=FlyHE`, `$HEart target=flyfhe`。
- 在根目录 `README.md` 新增 `Target Interface`，说明 OpenFHE/Lattigo/FlyHE(flyfhe) 三类目标别名。
- 将 `HEart/agents/openai.yaml` 纳入 skill 目录，提供 UI metadata 和默认 prompt。
- 安装 HEart skill 到本机 Codex skills 目录，便于重启 Codex 后直接调用。

产出文件清单:
- HEart/SKILL.md
- HEart/agents/openai.yaml
- README.md
- HEart/CHANGELOG.md
- PROGRESS.md

遗留/不确定点:
- Codex 需要重启后才能加载新安装的 skill。

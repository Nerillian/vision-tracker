# 第二次作业 · 项目说明

## 0. 基本信息

- 姓名：冯宇凡
- 学号：2026930908
- 完成日期：【2026-11-11】
- 是否完成全部基础功能：是

## 1. 类设计

| 类名 | 职责 | 继承自 |
| --- | --- | --- |
| `TrackingBase` | 抽象基类。保存单个目标的全部状态：`target_data t_data`（当前状态 13 项）+ `last_Observation l_obs`（上一次观测的快照）；提供 `removedata` / `writedata` 状态更新、`tackle_lost` 丢失推进、`writesnapshot` 输出快照 | — |
| `ArmorTarget` | 普通装甲策略：丢失阈值 3；不维护角度，角度与角速度、预测角恒为 0 | `TrackingBase` |
| `SpinningTarget` | 旋转装甲策略：丢失阈值 5；用观测间隔差分角速度，并按帧间隔预测角度 | `TrackingBase` |
| `TargetManager` | 目标管理器。持有全部目标（`vector<unique_ptr<TrackingBase>>`），实现整帧校验、逐帧调度（创建 / 更新 / 丢失）、统计与 `Result` 组装 | — |

类之间的关系：

```text
                       TrackingBase  (抽象基类, virtual ~TrackingBase)
                   ┌──────────────────────────────────────────────┐
                   │ 状态: target_data t_data + last_Observation l_obs │
                   │ 纯虚: target_tackle(dt) / lostThreshold()     │
                   └───────────────────┬──────────────────────────┘
                          ┌────────────┴────────────┐
                    ArmorTarget               SpinningTarget
                 (阈值 3, 角度恒 0)        (阈值 5, 角速度 + 角度预测)
                          └────────────┬────────────┘
                                       │ vector<unique_ptr<TrackingBase>>
                                 TargetManager
```

设计要点：

- 位置、速度、预测、丢失计数这些**两种装甲完全一致**的逻辑放在基类；
- 只有"阈值"和"角度"两处不同，用 `lostThreshold()` / `target_tackle()` 两个虚函数把差异下放到派生类；
- 基类析构声明为 `virtual`，因为容器用基类指针保存派生对象，删除时必须先执行派生类析构。

## 2. 一帧处理流程

按实际执行顺序简述 `onFrame`：

1. 清空本帧的"已观测编号"列表 `tackled`，构造 `Result{}` 并写入本帧帧号。
2. 计算帧间隔 `dt = (frame.timestamp_ns - last_frame_.last_timestamp_ns) / 1e9`（秒）。
3. `validate(frame)` 做整帧校验：帧号必须递增、时间戳必须递增、无负数编号、同帧编号不重复、已创建目标的类型必须与观测一致。
4. 校验通过后，才把 `last_frame_` 的帧号与时间戳更新为本帧的值。
5. 遍历本帧观测，按目标当前状态分三种情况：
   - 已存在且为 `TRACK`：`removedata` 先把旧状态存入 `l_obs`，再写入新观测；随后 `target_tackle` 用该目标自己的观测间隔 `dt_obs` 差分速度，并按帧间隔预测下一帧位置；
   - 已存在且为 `LOST`：`writedata` 用观测恢复状态（回到 `TRACK`、`missed_count` 清零、速度清零、预测值取观测值），再 `target_tackle`；
   - 不存在：`createTarget` 按观测类型 `new` 出 `ArmorTarget` 或 `SpinningTarget`（构造函数完成初始状态），`writedata` 后加入容器。
   每条观测都把编号记入 `tackled`。
6. 遍历容器中**本帧未被观测到**的目标，调用 `tackle_lost`：`missed_count` 自增；达到阈值（普通 3 / 旋转 5）则转为 `LOST` 并清零位置、角度、速度与预测值；未达阈值则按帧间隔推进预测值（旋转装甲同时推进角度）。
7. `fillResult` 把每个目标转成 `TargetSnapshot` 放入 `result.targets`，并统计 TRACK / LOST 数量与 ARMOR / SPINNING 数量。

**整帧校验失败时为什么不会留下部分修改：**

- 校验阶段全部是只读操作：只读 `frame` 的字段、只在容器里按编号查找并比较类型，不写任何成员；
- `onFrame` 中所有会产生副作用的语句（更新 `last_frame_`、目标状态更新、创建目标、丢失推进）都放在 `validate` 返回 `true` 之后，拒绝分支在更新基准之前就返回；
- 拒绝分支只做两件事：把 `result.accepted` 置 `false`，再调用 `fillResult` 把**当前已有状态**原样导出成快照。因此被拒绝的帧输出的目标状态与上一帧完全一致。

## 3. 附加项

### 3.1 完成情况

| 附加项 | 是否完成 |
| --- | --- |
| 附加一：`LOST` 目标超时删除 | 未完成 |
| 附加二：类型变化时重建目标 | 未完成 |
| 附加三：自主学习并使用 static | 未完成 |
| 附加四：自主学习并使用智能指针 | 已完成 |
| 附加五：目标排序 | 已完成 |

### 3.2 附加三：静态成员与 static

本次未使用 `static`（标记为未完成）。实现中的共享配置只有两个丢失阈值，目前分别写在 `ArmorTarget::lostThreshold()` 与 `SpinningTarget::lostThreshold()` 的返回值里，属于"每个类型一个常量"，不需要跨对象共享的静态状态。

### 3.3 附加四：智能指针

- **使用的类型**：`std::unique_ptr<TrackingBase>`。
- **使用位置**：
  - `TargetManager` 的成员 `std::vector<std::unique_ptr<TrackingBase>> targets_`；
  - `TargetManager::createTarget` 中用 `std::make_unique<ArmorTarget>` / `std::make_unique<SpinningTarget>` 创建对象；
  - `TargetManager::findtarget` 返回 `std::unique_ptr<TrackingBase>::get()` 得到的裸指针，用于调用成员函数（不转移所有权）。
- **选择理由**：
  1. 目标对象的真实类型在运行时才确定，容器必须存**指针**，否则按值存放会发生对象切片，派生类的虚函数失效；
  2. 目标是"由管理器创建、由管理器销毁、不共享给外部"的独占资源，`unique_ptr` 恰好表达"唯一所有权"；
  3. 对象在逐帧处理中会持续存在，不能放在栈上，用 `unique_ptr` 可以省掉手写的 `delete` 循环，异常安全且不会二次释放。
- **带来的好处**：`TargetManager` 不需要自定义析构函数去遍历删除；即使后续增加"类型变化时重建目标"之类的逻辑，只要 `reset()` 或 `erase`，旧对象也会被正确析构。
- **与虚析构的配合**：`TrackingBase` 的析构函数声明为 `virtual`，这样 `unique_ptr<TrackingBase>` 在销毁时才会调用派生类的析构函数，不会泄漏派生类部分。
- **不使用 `shared_ptr` 的原因**：目标对象没有任何共享所有权的场景，引用计数会带来额外的控制块开销和维护成本。
- **认为不需要智能指针的位置**：`onFrame(const InputFrame&)` 等接口参数只是"借用"外部数据，不拥有其生命周期，用引用即可；`ArmorTarget` / `SpinningTarget` 内部的状态成员是值类型，随对象生命周期自动管理。

### 3.4 附加五：目标排序

- **实现位置**：`TargetManager::fillResult()` 中，`targets` 填充完成后统一排序。
- **排序键**：
  1. 状态：`TRACK` 排在 `LOST` 之前（`TargetStatus` 中 `LOST = 0`、`TRACK = 1`，因此按状态降序）；
  2. 状态相同时，按目标到原点的距离升序，比较时使用平方距离 `x*x + y*y`，避免开方且排序结果一致；
  3. 状态和距离都相同时，按编号升序。
- **说明**：`LOST` 目标的位置已按要求清零，因此其距离恒为 0，第二级排序实际只在 `TRACK` 目标之间起作用。

## 4. 遇到的问题 / 未完成的部分

- **遇到的问题和解决过程**：
  1. **不清楚如何实现动态创建对象**：一开始不清楚我该怎么样实现动态创建一堆对象（当时忘了有new，然后指针学的不好不太敢用），后通过询问学长和ai解决（。
  2. **上不去github**：我裸连连不上，之后找ai配了个梯子，登上去了）。
  3. **一开始比较迷茫**：刚开始的时候很迷茫，感觉不知道要写啥，后边我用ai帮我整理了一下（。
  4. **漏写了很多东西**：各种忘记，感觉每次写的时候都会忘掉之前没写完的部分，这个后期改的时候都补上了。
- **已知未完成或有缺陷的地方**：
  - 附加一（`LOST` 超时删除）、附加二（类型变化重建）、附加三（static）未实现；

## 5. 编译运行与验证结果

**环境**：Windows / WSL 均可，g++ 支持 C++17，CMake ≥ 3.16。

```bash
cmake -S . -B build
cmake --build build
./build/tracker
```

或直接编译：

```bash
g++ -std=c++17 -Wall -Wextra -Iinclude -Iinclude/TargetManager \
    src/main.cpp src/TargetManager/*.cpp -o tracker
```

**输出规模**：28 帧，共 134 行（每帧 2 行统计 + 每个目标 1 行）。

**整帧校验结果**：共 5 帧被拒绝，与题目给定的一致：

| 帧序 | 原因 |
| --- | --- |
| 16（第一次） | 同一帧内编号 1 重复 |
| 19（第一次） | 编号为 −1 |
| 15（第二次） | 帧号未递增（回退到 15） |
| 23 | 目标 1 原为 ARMOR，被报成 SPINNING |
| 24 | 时间戳未递增（等于帧 22 的时间戳） |

**关键帧自测**（与题目第 10 节的自测清单逐条核对）：

| 帧 | 验证点 | 实际输出 |
| --- | --- | --- |
| 0–1 | 目标 1 速度与预测 | `velocity=(10.00,0.00)` `predicted_position=(3.00,2.00)` |
| 1 | 目标 2 角速度与预测角 | `angle=0.20` `angular_velocity=2.00` `predicted_angle=0.40` |
| 2–5 | 未观测时角度推进、位置预测保持 | 预测角 0.60 / 0.80 / 1.00 / 1.20，位置保持 `(5.00,0.00)` |
| 6 | 目标 1 跨 3 帧后速度；目标 2 转 LOST | `velocity=(3.33,0.00)` `predicted_position=(5.33,2.00)`；目标 2 全字段清零 |
| 7–8 | 未观测时预测值累加 | `(5.67,2.00)` → `(6.00,2.00)` |
| 12 | 普通装甲连续丢失 3 帧转 LOST | `status=0`，位置角度速度预测值全部为 0 |
| 13 | LOST 后重新追踪 | `status=1` `velocity=(0.00,0.00)` `predicted_position=(7.00,2.00)` |
| 20 | 目标 1、3、9 同时进入 LOST | `tracking_target_count=0 lost_target_count=4` |
| 21 | 旋转装甲重新追踪 | `angle=3.00` `angular_velocity=0.00` `predicted_angle=3.00` |
| 22 | 目标 9 重新追踪 | `predicted_position=(5.00,5.00)`；目标 2 预测角保持 `3.00` |

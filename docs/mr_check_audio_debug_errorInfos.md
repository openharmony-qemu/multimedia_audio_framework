**IssueNo**: #15325


**Description**: 
将 AudioFaultRecord 从 audio_debug_info.h 移至 audio_info.h，重命名 errorCodesInfo 为 errorInfos，
重构 AudioPolicyServer::GetAudioDebugInfo 复用调用者传入的 audioDebugInfo 对象。


**静态扫描截图**: 无新增告警


**通用规范自检：**
| 自检项                                                       | 自检结果  |
| ------------------------------------------------------------ | -------- |
| 成员变量命名，统一采用xxxx_                                      |NA|
| 换行优先使用4格缩进                                              |Y|
| *和&运算符统一跟随之后的变量                                      |Y|
| 避免使用未明确位宽的整型，选择使用int32_t, uint32_t等类型          |Y|
| 时间常量命名加后缀_MS, _US等，按照用途命名而非值                   |NA|

说明：
- 成员变量命名：本次新增的 `AudioFaultRecord` 为 POD struct，字段不属于类成员变量，不适用 xxxx_ 规范
- 整型位宽：`AudioFaultRecord` 使用 `int32_t uid`、`int32_t errorCode`、`uint64_t timestamp`
- 指针/引用风格：`std::vector<AudioFaultRecord> &errorInfos`，& 跟随变量名

**安全编码自检：**
| 自检项                                                          | 自检结果 |
| -------------------------------------------------------------- | -------- |
| 外部传入的指针使用前是否判空                                              |Y|
| 外部传入数据，直接用于内存申请、数组长度、索引、循环条件前，必须进行合法性校验 |Y|
| 成员或局部变量，尤其是指针、资源描述符、bool变量必须赋初值                   |Y|
| 内存分配后必须判断是否成功                                                |NA|
| 禁止打印内存地址、文件路径、设备名、MAC地址等敏感信息，如有需要，使用private修饰  |Y|
| 有符号、无符号整数之间**运算、比较**，确保不会出现溢出、反转、除0            |NA|
| 有无符号不允许混合计算，必须提前进行强转                                   |NA|
| map\vector\list\set等对象存在多线程并发访问场景必须加锁                    |NA|

说明：
- 外部指针判空：`audio_policy_server.cpp` 中 `if (!audioDebugInfo)` 判空后才使用；`interruptService_ != nullptr` 判空后才调用
- 外部数据校验：`GetErrorInfosFromMedia` 中字符串解析使用 `npos` 判定后才截取子串；数据来源为 `GetUnifiedFaultCodeRecords` 系统内部接口，非用户直接输入
- 变量赋初值：`AudioFaultRecord` 所有字段均有默认值（`uid=-1, errorCode=0, errorReason="", timestamp=0`）
- 内存分配：本次改动无 raw new，使用 `make_shared`，并在使用前判空
- 敏感信息：日志仅打印 `errorInfos size`、`sessionId`、`errorCode` 等非敏感调试信息
- 有无符号：本次改动无混合运算场景
- 并发加锁：`errorInfos` 在客户端进程内填充，不涉及多线程并发写入；`AudioDebugManagerPrivate` 的 map 访问已有 mutex 保护

**进程安全自检：**
| 自检项                                                          | 自检结果 |
| -------------------------------------------------------------- | -------- |
| SA新增idl接口，需要根据功能控制可访问范围，通过权限、UID校验等方式校验       |NA|
| SA新增idl接口，必须同时新增fuzz用例，并进行fuzz测试                        |NA|
| 类反序列化结果用于内存申请、数组长度、循环条件前，必须进行合法性校验          |NA|

说明：
- 未新增 IDL 接口，`GetAudioDebugInfo` 为已有接口
- `errorInfos` 字段不参与 IPC 序列化（客户端侧填充，不走 Marshalling/Unmarshalling），无反序列化风险

**XTS Result**: NA

**TDD Result**: NA（本次为结构体重构+重命名，无新逻辑分支）

**Fuzz Result**: NA（未新增 IDL 接口，无需新增 fuzz 用例）


# 计划

## 001. 巨型血条

- 模仿护盾Pips.Shield标签指定使用哪1帧或哪3帧，可填写1个值或3个值
  - Open `20220418`
  - Closed `20220419` 增加4个标签，但要求每个标签要么不写要么写满3个值

- 启用shp血条，读档丢失所有大血条信息包括Text数显，需要修复bug
  - Open `20220419`
  - Update `20220419` 可能是全局指针需换成静态成员，或者换成逐帧读shp文件
  - Closed `20220419` ststl tql!

- 护盾数值为0时是否移除护盾框、护盾数显，计划新增2个标签
  - Open `20220419`
  - Closed `20220419` 已新增2个标签

- 代码里为先画格子后画框，框的不透明颜色覆盖了格子
  - Open `20220419` 计划交换绘制次序，或者检测到反向标签时重新绘制一遍
  - Closed `20220419` 已新增1个反向标签，检测到反向标签重新绘制一遍框

- PipsCount用于自动调整格子绘制起始位置，但每次调整后需重新调整PipsOffset，很麻烦
  - Open `20220419`

- 考虑增加4个vector3D标签，用于控制生命和护盾3种健康状态下的框和格子的透明度
  - Open `20220421`

- 考虑增加自定义血条、护盾条，自动读取shp文件总帧数，按血量百分比播放相应的1帧
  - Open `20220421`

## 002. 跟随屏幕的动画水印

- 考虑通过快捷键来控制是否显示
  - Open `20220421` 参考Dump Object Info #83 Add a config to control if the development commands should be enabled
  - Update `20220421` 参考Display Damage Dealt ##590 FlyingStrings / TransactMoney.Display / Display damage hotkey command
  - Closed `20220421` 已新增快捷键控制

- 考虑与超武联动，作为超武开启时的全屏特效
  - Open `20220421` 参考虚拟投放
  - Update `20220422` 参考FlyingStrings，GScreen钩子里GScreenDisplay::UpdateAll()逐帧刷新，虚拟投放钩子激发GScreenDisplay::Add
  - Update `20220422` 已完成雏形，原理如上一条，可读档，暂时固定播放450帧（可新增LoopCount标签）
  - Closed `20220423` 已经新增LoopCount标签，但前一状态读档正常，本状态读档丢失超武动画播放进度

- 排查读档丢失超武动画播放进度
  - Open `20220423`

- 考虑读取shp文件的宽和高，起始位置由屏幕左上角改为屏幕中心
  - Open `20220423`
  - Update `20220424` 超武播放动画水印已更新
  - Closed `20220426` 已全部改为屏幕中心

- 考虑与Techno受伤害联动
  - Open `20220423`
  - Closed `20220423` 参考FlyingStrings

- 考虑与Techno开火联动
  - Open `20220423`

- 考虑与Techno本身的存在联动
  - Open `20220424`

- 若与Techno受伤害、开火、本身存在都有联动，考虑新增[GScreenAnimType]注册表
  - Open `20220423`
  - Closed `20220426` 已新增注册表

- 注意到FlyingStrings的单位受伤数值文本偏移量DamageNumberOffset是写在pExt里而非dataItem里
  - Open `20220423`
  - Closed `20220426` 后期注意dataItem的项尽可能少

- 每个Type.cpp的LoadFromStream读取SHP和PAL之后是否需要写一句ExternVariableClass::LoadVariablesFromDir()
  - Open `20220423`

## 003. 跟随屏幕的弹头

- 考虑释放超武时在屏幕中央持续释放「万花筒写轮眼-天照」，弹头爆炸点/武器目标点自动跟随视角移动
  - Open `20220422` 参考FlyingStrings，GScreen钩子里GScreenDisplay::UpdateAll()逐帧刷新，虚拟投放钩子激发GScreenDisplay::Add
  - Update `20220423` 变更思路，逐帧对特定Techno进行Remove&Put或者微观定义超时空移动，开火位置很高从而可以启用AreaFire激光贴图
  - Update `20220424` 变更思路，变更效果，超武启动瞄准动画水印自动跟随视角移动并投放单位，快捷键修改全局变量，逐帧检查单位和全局变量，刷0x0建筑
  - Update `20220424` 若先按下快捷键将全局变量设为true，则超武启动瞬间就刷出0x0建筑，需进行修复使得刷建筑条件更苛刻
  - Update `20220426` 变更思路，变更效果，不再绑定单位，超武释放时GScreenCreate::Add，逐帧检测GScreenCreate的vector而非检测单位，释放超武时/成功投放建筑时全局变量改为false
  - Update `20220428` 新增冷却时间标签。整套流程目前不支持多个超武同时挂载，需要在快捷键能做到的事情上动心思优化算法

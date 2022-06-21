
# 新特性说明书

## 001. 巨型血条

- 用于chm说明书

```text

- 关于格子、框，已支持使用4个标签指定帧
  - 建议格子文件、框文件各6帧，前3帧为HP的绿、黄、蓝状态，后3帧为SP的绿、黄、蓝状态
  - 框文件也可以为2帧，第1帧为HP框，第2帧为SP框

- 当前巨型血条自动检测机制：
  - 格子文件检测帧数，小于3帧则选第1帧，3帧及以上则选第1、2、3帧
  - 框文件检测帧数，小于3帧则选第1帧，3帧及以上则选第1、2、3帧
- 当前巨型护盾条自动检测机制：
  - 格子文件检测帧数，1帧则选第1帧，2帧则选第2帧，6帧则选第4、5、6帧，3-5帧则选最后一帧
  - 框文件检测帧数，1帧则选第1帧，2帧则选第2帧，6帧则选第4、5、6帧，3-5帧则选最后一帧

[HugeBar]>HugeHP.CustomSHPShowBar= (boolean)
启用自定义SHP巨型血条，关闭普通SHP巨型血条，关闭矩形形式血条。默认否。
[HugeBar]>HugeHP.ShowCustomSHP= (filename, *including*the .shp extension)
多帧自定义巨型血条SHP文件。默认pipbrd.shp。
[HugeBar]>HugeHP.ShowCustomPAL= (filename, *including*the .pal extension)
自定义巨型血条色盘。默认palette.pal。
[HugeBar]>HugeSP.CustomSHPShowBar= (boolean)
启用自定义SHP巨型护盾条，关闭普通SHP巨型护盾条，关闭矩形形式护盾条。默认否。
[HugeBar]>HugeSP.ShowCustomSHP= (filename, *including*the .shp extension)
多帧自定义巨型护盾条SHP文件。默认pipbrd.shp。
[HugeBar]>HugeSP.ShowCustomPAL= (filename, *including*the .pal extension)
自定义巨型护盾条色盘。默认palette.pal。

```

- 用于测试

```ini

[HugeBar]
HugeHP.UseSHPShowBar=true ;同时启用SHP巨型血条、SHP巨型护盾条，关闭矩形形式血条护盾条
HugeHP.ShowBarSHP= ;血条框，默认pipbrd.shp
HugeHP.ShowBarPAL= ;血条框色盘，默认palette.pal
HugeHP.ShowPipsSHP= ;血条格子，默认pips.shp
HugeHP.ShowPipsPAL=test.pal ;血条格子色盘，默认palette.pal

HugeHP.BarFrames= ;血条框的帧序号，必须写满3个值，如0,0,0或者0,1,2，默认值见检测机制
HugeHP.PipsFrames= ;血条格子的帧序号，必须写满3个值，如0,0,0或者0,1,2，默认值见检测机制
HugeSP.BarFrames= ;护盾框的帧序号，必须写满3个值，如1,1,1或者3,4,5，默认值见检测机制
HugeSP.PipsFrames= ;护盾格子的帧序号，必须写满3个值，如3,3,3或者3,4,5，默认值见检测机制

HugeHP.PipsCount= ;满血时的格子数量，默认为100，每次修改必须重新调整PipsOffset
HugeHP.PipWidth= ;格子宽度，默认为HugeHP.ShowPipsSHP的shp文件宽度
HugeHP.PipsOffset= ;所有格子位置同步调整，默认(0,0)
HugeHP.PipToPipOffset= ;每个格子相对于前一个格子的偏移量，默认(0,0)，注意是基于(HugeHP.PipWidth,0)

HugeSP.PipsCount= ;护盾满血时的格子数量，默认为100，每次修改必须重新调整PipsOffset
HugeSP.PipWidth= ;格子宽度，默认为HugeHP.ShowPipsSHP的shp文件宽度
HugeSP.PipsOffset= ;所有格子位置同步调整，默认(0,0)
HugeSP.PipToPipOffset= ;每个格子相对于前一个格子的偏移量，默认(0,0)，注意是基于(HugeHP.PipWidth,0)

HugeSP.BarFrameEmpty= ;护盾值为0时的护盾框的帧序号，默认为-1，即不显示护盾框
HugeSP.ShowValueAlways= ;护盾值为0时，是否显示护盾数值，默认为否
HugeHP.DrawOrderReverse= ;是否先绘制格子再绘制框，用于框内有不规则图形遮盖格子的特殊情形，默认否（即先绘制框后绘制格子，格子覆盖框）

HugeHP.CustomSHPShowBar= ;启用自定义SHP巨型血条，关闭普通SHP巨型血条，关闭矩形形式血条
HugeHP.ShowCustomSHP= ;多帧自定义巨型血条SHP文件，默认pipbrd.shp
HugeHP.ShowCustomPAL= ;色盘，默认palette.pal

HugeSP.CustomSHPShowBar= ;启用自定义SHP巨型护盾条，关闭普通SHP巨型护盾条，关闭矩形形式护盾条
HugeSP.ShowCustomSHP= ;多帧自定义巨型护盾条SHP文件，默认pipbrd.shp
HugeSP.ShowCustomPAL= ;色盘，默认palette.pal

```

## 002. 跟随屏幕的动画水印

- 用于chm说明书

```text

现在可以在游戏全程/超武释放时/科技类型受伤害时播放指定的shp文件作为动画水印，动画自动跟随屏幕即玩家视角移动。
若为游戏全程的动画水印，则动画无限循环播放，可设置快捷键进行开启/关闭。
若为超武释放时/单位受伤害时的动画水印，则播放途中存档无法存入当前动画进度，读档后无动画。

注意：动画水印的类型需要新增一个注册表[GScreenAnimTypes]并在里面注册。

[AudioVisual]>GScreenAnimType= (GScreenAnimType)
游戏全程使用的动画水印类型。若启用，可在[游戏控制]>[键盘]>[扩展功能]>[Display Animation]中指定快捷键来即时切换是否显示动画。
[SuperWeaponType]>GScreenAnimType= (GScreenAnimType)
超武释放时启用的动画水印类型。
[TechnoType]>GScreenAnimType= (GScreenAnimType)
科技类型受伤害时启用的动画水印类型。

[GScreenAnimType]>ShowAnim.SHP= (filename, *including*the .shp extension)
动画水印使用的文件。默认pips.shp。
[GScreenAnimType]>ShowAnim.PAL= (filename, *including*the .pal extension)
动画水印使用的色盘。默认anim.pal。
[GScreenAnimType]>ShowAnim.Offset= (X,Y)
调整动画水印基于屏幕正中央的显示位置。负数左/上，正数右/下。默认0,0。
[GScreenAnimType]>ShowAnim.TranslucentLevel= (integer)
动画水印的透明度，0为0%，1为25%，2为50%，3为75%，越大越透明。默认0。
[GScreenAnimType]>ShowAnim.FrameKeep= (integer)
每帧动画实际使用多少帧时间播放。默认5。
[GScreenAnimType]>ShowAnim.LoopCount= (integer)
动画循环次数。默认1。游戏全程动画水印无限循环。
[GScreenAnimType]>ShowAnim.CoolDown= (integer)
受伤害后多少帧内再次受伤害不会新增动画，用于避免频繁新增动画导致掉帧，默认100。

```

- 用于测试

```ini

[GScreenAnimTypes]
0=GlobalAnim
1=SWDeliveryAnim
2=HandDamageAnim
3=WeapDamageAnim

[AudioVisual]
GScreenAnimType=GlobalAnim

[SuperWeaponType]
GScreenAnimType=SWDeliveryAnim

[NAHAND]
GScreenAnimType=HandDamageAnim

[NAWEAP]
GScreenAnimType=WeapDamageAnim

[HandDamageAnim]
ShowAnim.SHP=BDD.shp ;默认pips.shp
ShowAnim.PAL=anim.pal ;默认anim.pal
ShowAnim.Offset=0,550 ;起始位置，默认(0,0)，即屏幕正中央
ShowAnim.TranslucentLevel=2 ;透明度0-3，默认为0，即不透明
ShowAnim.FrameKeep=10 ;每帧SHP实际使用多少帧时间播放，默认为5
ShowAnim.LoopCount=2 ;动画循环次数，默认1
ShowAnim.CoolDown=200 ;受伤害后多少帧内再次受伤害不会新增动画，默认100

```

## 003. 使用快捷键自动投放建筑到屏幕中央

- 用于chm说明书

```text

现在可以在超武释放后一段时间内每次按下键盘快捷键时自动投放1个建筑到屏幕中央，建筑需自定义面积0x0以避免弹窗。
快捷键位于[游戏控制]>[键盘]>[扩展功能]>[Create Building]。
建筑绑定的超级武器有效。
在持续时间内存档，读档之后立即按下快捷键，将弹窗。
目前不支持多个超武同时挂载此功能。

[SuperWeaponType]>CreateBuilding= (boolean)
是否在此超武释放后Duration指定的时间内每次按下键盘快捷键时自动投放1个0x0建筑到屏幕中央。默认否。
[SuperWeaponType]>CreateBuilding.Type= (BuildingType)
投放的建筑的注册名。
[SuperWeaponType]>CreateBuilding.Duration= (integer)
该效果持续的游戏帧数。默认1500。
[SuperWeaponType]>CreateBuilding.Reload= (integer)
每次投放建筑后多少帧内无法响应投放指令。默认100。
[SuperWeaponType]>CreateBuilding.AutoCreate= (boolean)
是否以Reload为间隔自动连续投放建筑。若启用，可在[游戏控制]>[键盘]>[扩展功能]>[Create Building Automatically]中指定快捷键来即时切换连续自动开火状态和停火状态。默认否。

```

- 用于测试

```ini

[SuperWeaponType]
CreateBuilding= ;是否在此超武释放后一段时间内每次按下键盘快捷键时自动投放1个0x0建筑到屏幕中央，默认否
CreateBuilding.Type= ;投放的建筑的注册名
CreateBuilding.Duration= ;该效果持续的游戏帧数。默认1500。
CreateBuilding.Reload= ;每次投放建筑后多少帧内无法响应投放指令。默认100
CreateBuilding.AutoCreate= ;是否以Reload为间隔自动连续投放建筑

```

## 004. 修复相同国家争抢计分超武的问题

- 用于chm说明书

```text

[Country]>ScoreSuperWeapon.OnlyOnce= (boolean)
不同参战方使用相同国家（即子阵营）时，当其中一个参战方率先满足得分条件获得超武，其他所有参战方后续满足得分条件是否无法获得超武。
默认否，即任意参战方只要满足分数条件必能获得超武。

```

## 005. JumpJet部署变形

- 用于chm说明书

```text

当卸载乘员时变形为指定单位（主要用于实现类似RN的直升机）

由MOD《世纪之战》提供：https://www.bilibili.com/read/cv15730238

代码来自：https://github.com/ChrisLv-CN/YRDynamicPatcher-Kratos/pull/4

十分感谢何老师授权移植！

[JumpJet_Float]
IsSimpleDeployer=yes
DeployToLand=yes
GroupAs=JumpJet_Float
BalloonHover=yes
JJConvert.Unload=JumpJet_Land

[JumpJet_Land]
BalloonHover=no
GroupAs=JumpJet_Float

```

## 006. 染色弹头

- 用于chm说明书

```text

现在可以通过弹头来为目标染色了，染色持续时间大于0且弹头对目标护甲伤害比例大于0%时启用此功能，暂不支持飞行器染色。

[Warhead]>PaintBall.Duration= (integer - frames)
染色的持续时间。默认0。
[Warhead]>PaintBall.Color= (R,G,B)
弹头命中后对目标的染色。默认255,0,0。

```

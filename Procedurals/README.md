## Procedurals
----------------------------------------------
### Speedtree Kit

![sp_2](https://user-images.githubusercontent.com/16664056/44787787-ae406780-abca-11e8-9b1e-35f288dc3734.png)
1. **Overview and Purpose**

     The [SpeedtreeKit](https://github.com/iceprincefounder/selected-sources/tree/master/Procedurals/speedtreeKit) is a    toolset which contents speedtreeLib(based on Unreal Engine), [speedtreeProc](https://github.com/iceprincefounder/selected-sources/tree/master/Procedurals/speedtreeKit/speedtreeProc) and [speedtreeOp](https://github.com/iceprincefounder/selected-sources/tree/master/Procedurals/speedtreeKit/speedtreeOp). The speedtreeLib is a speedtree translation lib would convert speedtree original geometry data into general purpose vertex index vector so that the other applications could use. The speedtreeProc contents arnold procedural node " speedtree_procedural" and a shader "speedtree_shader", the procedural node would read speedtree original file which suffix is *.srt via translation lib\`s API and convert into renderer polymesh node, it might generate more than one polymesh node would be assigned with a "speedtree_shader" to make sure every polymesh node would get different shader and different look. The speedtreeOp contents a Katana Op node "Speedtree_In" which would read original speedtree file into Katana and available to render and also create a proxy alembic file in the viewer.
 
 2. **Pipeline Design**

    Use "Speedtree_In" node to read original speedtree files and create the looks with "speedtree_shader" then export assets into arnold ASS files and go back to maya and use xgen to create the plant in shot.Finally, render the shot assets in Katana with "xgen_procedural" node.
 
 3. **Why use speedtreeKit**

    The speedtreeKit toolset would make your assets of plant very small and don\`t need the animation sequences because every animation frame would be calculated by the renderer. That would improve both your IO speed and also render speed. 

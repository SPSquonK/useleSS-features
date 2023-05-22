# Useless - Sphere Grid

Based on [useleSS commit 5485bc6c9215aa1a230bd96ff9d141dfc0f366b3](https://github.com/SPSquonK/useleSS/tree/5485bc6c9215aa1a230bd96ff9d141dfc0f366b3).

## Concept

- Replace the whole simple "stat increase" with a complex grid inspired by FFX
Sphere grid.
- Modify the skill window to only display skills to let the user use them
- Everyone is a vagrant


## I'm a youtuber

![EditingDemo.mp4](EditingDemo.mp4)

## TODO

- Window
  - Editing tab layout
  - Grid progression
    - Design the tab for the player
    - Users can use their stats points in the grid
    - Increase several skills at once (yay path finding)
  - Build a grid for demonstration purposes
  - Put the differents classes in proper files
  - Redesign the skill window ([probably reuse SFlyFF 2 layout](http://sflyff.fr/img/cumuldeclasses4.png))
- Vagrant Story: Nuke all classes progression / restriction
- Networking briks
  - Send / receive full grid
  - Send / receive grid progression request
- Worldserver
  - Players use sphere grid for stats computation and skill checking
  - Networking logic
- Database server
  - Persistence of grid progression

## ODBC
- ODBC source `useless_sg_account` = table `USELESS_SG_ACCOUNT_DBF`
- ODBC source `useless_sg_character` = table `USELESS_SG_CHARACTER_01_DBF`
- ODBC source `useless_sg_log` = table `USELESS_SG_LOGGING_01_DBF`

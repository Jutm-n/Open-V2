# Open-V2 - Installation guide

#### 1
Update your visual studio to latest in visual studio installer (!Important)
I use visual studio 2017, any later should work
#### 2
Copy mod/OpenV2 to your Vic2 folder
#### 3
Build container_generator, gui_window_generator & parser_generator
#### 4
Open file_explorer.cpp, go to line 222,
change 'E:\\programs\\Victoria2' to the path to your Vic2 folder
make a new folder anywhere and set 'E:\\programs\\V2_scenario' to the path
#### 5
Set file_explorer to startup project and run
If it doesn't work try running again
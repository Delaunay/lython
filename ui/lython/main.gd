extends Panel


func set_layout_size():
	var size = $Page/Body.size

	var left_split = $Page/Body/LeftSplit
	var left_panel = $Page/Body/LeftSplit/LeftPanel
	
	var right_split = $Page/Body/LeftSplit/RightSplit
	var main = $Page/Body/LeftSplit/RightSplit/TabContainer

	var pct = 0.2
	var main_size = size.x * (1 - pct * 2)
	var panel_size = size.x * pct

	# Works more or less
	left_split.split_offset = -left_panel.size.x + panel_size
	right_split.split_offset = -main.size.x + main_size


func _ready():
	set_layout_size()

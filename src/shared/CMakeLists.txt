project(shared)

set(IMGUI_DIR ${PROJECT_SOURCE_DIR}/../../thirdparty/imgui-1.87/)

set(IMGUI_SOURCES
	imgui.cpp
	imgui_demo.cpp
	imgui_draw.cpp
	imgui_tables.cpp
	imgui_widgets.cpp
	imconfig.h
	imgui.h
	imgui_internal.h
	imstb_rectpack.h
	imstb_textedit.h
	imstb_truetype.h

	backends/imgui_impl_win32.cpp
	backends/imgui_impl_opengl3.cpp
	backends/imgui_impl_win32.h
	backends/imgui_impl_opengl3.h
)

list(TRANSFORM IMGUI_SOURCES PREPEND ${IMGUI_DIR})

add_library(imgui STATIC)
target_sources(imgui PRIVATE ${IMGUI_SOURCES})
target_include_directories(imgui PUBLIC ${IMGUI_DIR})

set(SOURCES
	alloc.cpp
	anim_track.cpp
	animation.cpp
	animation_manager.cpp
	asset.cpp
	axes.cpp
	bag.cpp
	board.cpp
	byte_reader.cpp
	byte_writer.cpp
	chat.cpp
	colour.cpp
	console.cpp
	convar.cpp
	commandline.cpp
	debug.cpp
	engine.cpp
	font.cpp
	font_bitmap.cpp
	font_manager.cpp
	font_ttf.cpp
	frustum.cpp
	game.cpp
	gfx_io.cpp
	hash_murmur3.cpp
	hash_sha256.cpp
	ieee754.cpp
	imgui_utils.cpp
	io.cpp
	kickset_manager.cpp
	kickset_parse.cpp
	lodepng.cpp
	maths.cpp
	mesh.cpp
	mesh_manager.cpp
	mesh_skeletal.cpp
	mesh_static.cpp
	mesh_vertex.cpp
	mutex.cpp
	net.cpp
	net_address.cpp
	net_packet.cpp
	net_socket.cpp
	net_tcp_digest.cpp
	net_utils.cpp
	objreader.cpp
	phys_body.cpp
	phys_collider.cpp
	phys_collider_algorithms.cpp
	phys_collision.cpp
	phys_collision_box.cpp
	phys_collision_capsule.cpp
	phys_collision_sphere.cpp
	phys_simulation.cpp
	player.cpp
	projection.cpp
	property_collection.cpp
	puyoboard.cpp
	quaternion.cpp
	random.cpp
	server.cpp
	shaperotator.cpp
	shared_ptr.cpp
	sheetformat_manager.cpp
	sheetformat_parse.cpp
	sproperty_collection.cpp
	string_utils.cpp
	sys.cpp
	tetrisboard.cpp
	text.cpp
	text_manager.cpp
	text_provider.cpp
	timer.cpp
	transform.cpp
	user.cpp
	utils.cpp
	vector.cpp
	volume.cpp
)

set(HEADERS
	alloc.h
	allocator.h
	anim_joint.h
	anim_keyframe.h
	anim_skeleton.h
	anim_track.h
	animation.h
	animation_manager.h
	assert.h
	asset.h
	asset_manager.h
	axes.h
	bag.h
	board.h
	box.h
	byte_reader.h
	byte_writer.h
	chat.h
	colour.h
	concepts.h
	console.h
	convar.h
	commandline.h
	cursor.h
	debug.h
	defs.h
	engine.h
	event.h
	exceptions.h
	font.h
	font_bitmap.h
	font_manager.h
	font_ttf.h
	frustum.h
	function.h
	function_member.h
	game.h
	gfx_io.h
	glyph.h
	hash_functor.h
	hash_murmur3.h
	hash_sha256.h
	ieee754.h
	imgui_utils.h
	io.h
	iterator_utils.h
	keycode.h
	kickset.h
	kickset_manager.h
	line_segment.h
	lodepng.h
	macro_utils.h
	maths.h
	mesh.h
	mesh_manager.h
	mesh_skeletal.h
	mesh_static.h
	mesh_vertex.h
	mutex.h
	net.h
	net_address.h
	net_filter.h
	net_packet.h
	net_protocol.h
	net_socket_tcp.h
	net_socket_udp.h
	net_tcp_digest.h
	net_utils.h
	phys_body.h
	phys_collider.h
	phys_collision.h
	phys_collision_box.h
	phys_collision_capsule.h
	phys_collision_channels.h
	phys_collision_shape.h
	phys_collision_sphere.h
	phys_raycast_hit_info.h
	phys_simulation.h
	player.h
	projection.h
	property.h
	property_collection.h
	puyoboard.h
	quaternion.h
	random.h
	ray.h
	rect.h
	rotation.h
	server.h
	shapedef.h
	shaperotator.h
	shared_ptr.h
	sheetformat.h
	sheetformat_manager.h
	sorting.h
	sproperty.h
	sproperty_collection.h
	string.h
	string_utils.h
	sys.h
	t_array.h
	t_buf.h
	t_hashmap.h
	t_idxset.h
	t_list.h
	t_map.h
	t_mat4.h
	t_matrix.h
	t_memory.h
	t_paged_memory.h
	t_pair.h
	t_stack.h
	t_vec.h
	t_vec2.h
	t_vec3.h
	tetrisboard.h
	text.h
	text_manager.h
	text_provider.h
	timer.h
	transform.h
	types.h
	user.h
	utils.h
	vec4.h
	volume.h
)

set(TARGETNAME ${PROJECT_NAME}_interface)
add_library(${TARGETNAME} INTERFACE)
target_sources(${TARGETNAME} INTERFACE ${SOURCES} ${HEADERS})
target_include_directories(${TARGETNAME} INTERFACE ${PROJECT_SOURCE_DIR}/.. ${PROJECT_SOURCE_DIR}/../../include)
target_link_libraries(${TARGETNAME} INTERFACE imgui freetype Ws2_32)
target_compile_definitions(${TARGETNAME} INTERFACE WIN32_LEAN_AND_MEAN)
add_library(gcpt::shared ALIAS ${TARGETNAME})

add_custom_target(${PROJECT_NAME} SOURCES ${SOURCES} ${HEADERS})

function(gcpt_shared_set_source_groups)
	get_target_property(GCPT_SHARED_SRC gcpt::shared INTERFACE_SOURCES)
	set(GCPT_SHARED_SOURCES ${GCPT_SHARED_SRC})
	set(GCPT_SHARED_HEADERS ${GCPT_SHARED_SRC})

	set(_header_regex ".*\\.([Hh]|[Hh][Pp][Pp])")
	list(FILTER GCPT_SHARED_SOURCES EXCLUDE REGEX ${_header_regex})
	list(FILTER GCPT_SHARED_HEADERS INCLUDE REGEX ${_header_regex})

	source_group("Shared Header Files" FILES ${GCPT_SHARED_HEADERS})
	source_group("Shared Source Files" FILES ${GCPT_SHARED_SOURCES})
endfunction()

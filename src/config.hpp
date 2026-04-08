/**
 * @file config.hpp
 * @author Jaroslav Hucel (xhucel00@vutbr.cz)
 * @brief TODO:
 *
 * @date Created:  25. 02. 2026
 * @date Modified: 26. 03. 2026
 *
 * @copyright Copyright (c) 2025 -> Public Domain, for more information see LICENSE
 */

#pragma once
#include <cstdint>
#include <vector>

enum class LogLevel : uint8_t {
    None,
    Error,
    Warning,
    All,
};

enum class DeprecationBehavior : uint8_t {
    GenerateWithDeprecationWarning,
    GenerateWithoutDeprecationWarning,
    DontGenerateDeprecatedTypes,
};

enum class Compact : uint8_t {
    Normal,
    Compact,
    Padded
};

enum class VulkanVersion : uint8_t {
    VK_VERSION_1_0,
    VK_VERSION_1_1,
    VK_VERSION_1_2,
    VK_VERSION_1_3,
    VK_VERSION_1_4,
};

#define EXTENSION_LIST(X) \
    X(VK_KHR_surface) \
    X(VK_KHR_swapchain) \
    X(VK_KHR_display) \
    X(VK_KHR_display_swapchain) \
    X(VK_KHR_xlib_surface) \
    X(VK_KHR_xcb_surface) \
    X(VK_KHR_wayland_surface) \
    X(VK_KHR_mir_surface) \
    X(VK_KHR_android_surface) \
    X(VK_KHR_win32_surface) \
    X(VK_ANDROID_native_buffer) \
    X(VK_EXT_debug_report) \
    X(VK_NV_glsl_shader) \
    X(VK_EXT_depth_range_unrestricted) \
    X(VK_KHR_sampler_mirror_clamp_to_edge) \
    X(VK_IMG_filter_cubic) \
    X(VK_AMD_extension_17) \
    X(VK_AMD_extension_18) \
    X(VK_AMD_rasterization_order) \
    X(VK_AMD_extension_20) \
    X(VK_AMD_shader_trinary_minmax) \
    X(VK_AMD_shader_explicit_vertex_parameter) \
    X(VK_EXT_debug_marker) \
    X(VK_KHR_video_queue) \
    X(VK_KHR_video_decode_queue) \
    X(VK_AMD_gcn_shader) \
    X(VK_NV_dedicated_allocation) \
    X(VK_EXT_extension_28) \
    X(VK_EXT_transform_feedback) \
    X(VK_NVX_binary_import) \
    X(VK_NVX_image_view_handle) \
    X(VK_AMD_extension_32) \
    X(VK_AMD_extension_33) \
    X(VK_AMD_draw_indirect_count) \
    X(VK_AMD_extension_35) \
    X(VK_AMD_negative_viewport_height) \
    X(VK_AMD_gpu_shader_half_float) \
    X(VK_AMD_shader_ballot) \
    X(VK_KHR_video_encode_h264) \
    X(VK_KHR_video_encode_h265) \
    X(VK_KHR_video_decode_h264) \
    X(VK_AMD_texture_gather_bias_lod) \
    X(VK_AMD_shader_info) \
    X(VK_AMD_extension_44) \
    X(VK_KHR_dynamic_rendering) \
    X(VK_AMD_extension_46) \
    X(VK_AMD_shader_image_load_store_lod) \
    X(VK_NVX_extension_48) \
    X(VK_GOOGLE_extension_49) \
    X(VK_GGP_stream_descriptor_surface) \
    X(VK_NV_corner_sampled_image) \
    X(VK_NV_private_vendor_info) \
    X(VK_NV_extension_53) \
    X(VK_KHR_multiview) \
    X(VK_IMG_format_pvrtc) \
    X(VK_NV_external_memory_capabilities) \
    X(VK_NV_external_memory) \
    X(VK_NV_external_memory_win32) \
    X(VK_NV_win32_keyed_mutex) \
    X(VK_KHR_get_physical_device_properties2) \
    X(VK_KHR_device_group) \
    X(VK_EXT_validation_flags) \
    X(VK_NN_vi_surface) \
    X(VK_KHR_shader_draw_parameters) \
    X(VK_EXT_shader_subgroup_ballot) \
    X(VK_EXT_shader_subgroup_vote) \
    X(VK_EXT_texture_compression_astc_hdr) \
    X(VK_EXT_astc_decode_mode) \
    X(VK_EXT_pipeline_robustness) \
    X(VK_KHR_maintenance1) \
    X(VK_KHR_device_group_creation) \
    X(VK_KHR_external_memory_capabilities) \
    X(VK_KHR_external_memory) \
    X(VK_KHR_external_memory_win32) \
    X(VK_KHR_external_memory_fd) \
    X(VK_KHR_win32_keyed_mutex) \
    X(VK_KHR_external_semaphore_capabilities) \
    X(VK_KHR_external_semaphore) \
    X(VK_KHR_external_semaphore_win32) \
    X(VK_KHR_external_semaphore_fd) \
    X(VK_KHR_push_descriptor) \
    X(VK_EXT_conditional_rendering) \
    X(VK_KHR_shader_float16_int8) \
    X(VK_KHR_16bit_storage) \
    X(VK_KHR_incremental_present) \
    X(VK_KHR_descriptor_update_template) \
    X(VK_NVX_device_generated_commands) \
    X(VK_NV_clip_space_w_scaling) \
    X(VK_EXT_direct_mode_display) \
    X(VK_EXT_acquire_xlib_display) \
    X(VK_EXT_display_surface_counter) \
    X(VK_EXT_display_control) \
    X(VK_GOOGLE_display_timing) \
    X(VK_RESERVED_do_not_use_94) \
    X(VK_NV_sample_mask_override_coverage) \
    X(VK_NV_geometry_shader_passthrough) \
    X(VK_NV_viewport_array2) \
    X(VK_NVX_multiview_per_view_attributes) \
    X(VK_NV_viewport_swizzle) \
    X(VK_EXT_discard_rectangles) \
    X(VK_NV_extension_101) \
    X(VK_EXT_conservative_rasterization) \
    X(VK_EXT_depth_clip_enable) \
    X(VK_NV_extension_104) \
    X(VK_EXT_swapchain_colorspace) \
    X(VK_EXT_hdr_metadata) \
    X(VK_IMG_extension_107) \
    X(VK_IMG_extension_108) \
    X(VK_KHR_imageless_framebuffer) \
    X(VK_KHR_create_renderpass2) \
    X(VK_IMG_relaxed_line_rasterization) \
    X(VK_KHR_shared_presentable_image) \
    X(VK_KHR_external_fence_capabilities) \
    X(VK_KHR_external_fence) \
    X(VK_KHR_external_fence_win32) \
    X(VK_KHR_external_fence_fd) \
    X(VK_KHR_performance_query) \
    X(VK_KHR_maintenance2) \
    X(VK_KHR_extension_119) \
    X(VK_KHR_get_surface_capabilities2) \
    X(VK_KHR_variable_pointers) \
    X(VK_KHR_get_display_properties2) \
    X(VK_MVK_ios_surface) \
    X(VK_MVK_macos_surface) \
    X(VK_MVK_moltenvk) \
    X(VK_EXT_external_memory_dma_buf) \
    X(VK_EXT_queue_family_foreign) \
    X(VK_KHR_dedicated_allocation) \
    X(VK_EXT_debug_utils) \
    X(VK_ANDROID_external_memory_android_hardware_buffer) \
    X(VK_EXT_sampler_filter_minmax) \
    X(VK_KHR_storage_buffer_storage_class) \
    X(VK_AMD_gpu_shader_int16) \
    X(VK_AMD_extension_134) \
    X(VK_AMDX_shader_enqueue) \
    X(VK_KHR_extension_136) \
    X(VK_AMD_mixed_attachment_samples) \
    X(VK_AMD_shader_fragment_mask) \
    X(VK_EXT_inline_uniform_block) \
    X(VK_AMD_extension_140) \
    X(VK_EXT_shader_stencil_export) \
    X(VK_KHR_shader_bfloat16) \
    X(VK_AMD_extension_143) \
    X(VK_EXT_sample_locations) \
    X(VK_KHR_relaxed_block_layout) \
    X(VK_RESERVED_do_not_use_146) \
    X(VK_KHR_get_memory_requirements2) \
    X(VK_KHR_image_format_list) \
    X(VK_EXT_blend_operation_advanced) \
    X(VK_NV_fragment_coverage_to_color) \
    X(VK_KHR_acceleration_structure) \
    X(VK_NV_extension_152) \
    X(VK_NV_framebuffer_mixed_samples) \
    X(VK_NV_fill_rectangle) \
    X(VK_NV_shader_sm_builtins) \
    X(VK_EXT_post_depth_coverage) \
    X(VK_KHR_sampler_ycbcr_conversion) \
    X(VK_KHR_bind_memory2) \
    X(VK_EXT_image_drm_format_modifier) \
    X(VK_EXT_extension_160) \
    X(VK_EXT_validation_cache) \
    X(VK_EXT_descriptor_indexing) \
    X(VK_EXT_shader_viewport_index_layer) \
    X(VK_KHR_portability_subset) \
    X(VK_NV_shading_rate_image) \
    X(VK_NV_ray_tracing) \
    X(VK_NV_representative_fragment_test) \
    X(VK_NV_extension_168) \
    X(VK_KHR_maintenance3) \
    X(VK_KHR_draw_indirect_count) \
    X(VK_EXT_filter_cubic) \
    X(VK_QCOM_render_pass_shader_resolve) \
    X(VK_QCOM_extension_173) \
    X(VK_QCOM_extension_174) \
    X(VK_EXT_global_priority) \
    X(VK_KHR_shader_subgroup_extended_types) \
    X(VK_EXT_extension_177) \
    X(VK_KHR_8bit_storage) \
    X(VK_EXT_external_memory_host) \
    X(VK_AMD_buffer_marker) \
    X(VK_KHR_shader_atomic_int64) \
    X(VK_KHR_shader_clock) \
    X(VK_AMD_extension_183) \
    X(VK_AMD_pipeline_compiler_control) \
    X(VK_EXT_calibrated_timestamps) \
    X(VK_AMD_shader_core_properties) \
    X(VK_AMD_extension_187) \
    X(VK_KHR_video_decode_h265) \
    X(VK_KHR_global_priority) \
    X(VK_AMD_memory_overallocation_behavior) \
    X(VK_EXT_vertex_attribute_divisor) \
    X(VK_GGP_frame_token) \
    X(VK_EXT_pipeline_creation_feedback) \
    X(VK_GOOGLE_extension_194) \
    X(VK_GOOGLE_extension_195) \
    X(VK_GOOGLE_extension_196) \
    X(VK_KHR_driver_properties) \
    X(VK_KHR_shader_float_controls) \
    X(VK_NV_shader_subgroup_partitioned) \
    X(VK_KHR_depth_stencil_resolve) \
    X(VK_KHR_swapchain_mutable_format) \
    X(VK_NV_compute_shader_derivatives) \
    X(VK_NV_mesh_shader) \
    X(VK_NV_fragment_shader_barycentric) \
    X(VK_NV_shader_image_footprint) \
    X(VK_NV_scissor_exclusive) \
    X(VK_NV_device_diagnostic_checkpoints) \
    X(VK_KHR_timeline_semaphore) \
    X(VK_KHR_extension_209) \
    X(VK_INTEL_shader_integer_functions2) \
    X(VK_INTEL_performance_query) \
    X(VK_KHR_vulkan_memory_model) \
    X(VK_EXT_pci_bus_info) \
    X(VK_AMD_display_native_hdr) \
    X(VK_FUCHSIA_imagepipe_surface) \
    X(VK_KHR_shader_terminate_invocation) \
    X(VK_GOOGLE_extension_217) \
    X(VK_EXT_metal_surface) \
    X(VK_EXT_fragment_density_map) \
    X(VK_EXT_extension_220) \
    X(VK_KHR_extension_221) \
    X(VK_EXT_scalar_block_layout) \
    X(VK_EXT_extension_223) \
    X(VK_GOOGLE_hlsl_functionality1) \
    X(VK_GOOGLE_decorate_string) \
    X(VK_EXT_subgroup_size_control) \
    X(VK_KHR_fragment_shading_rate) \
    X(VK_AMD_shader_core_properties2) \
    X(VK_AMD_extension_229) \
    X(VK_AMD_device_coherent_memory) \
    X(VK_AMD_extension_231) \
    X(VK_AMD_extension_232) \
    X(VK_KHR_dynamic_rendering_local_read) \
    X(VK_AMD_extension_234) \
    X(VK_EXT_shader_image_atomic_int64) \
    X(VK_KHR_shader_quad_control) \
    X(VK_KHR_spirv_1_4) \
    X(VK_EXT_memory_budget) \
    X(VK_EXT_memory_priority) \
    X(VK_KHR_surface_protected_capabilities) \
    X(VK_NV_dedicated_allocation_image_aliasing) \
    X(VK_KHR_separate_depth_stencil_layouts) \
    X(VK_INTEL_extension_243) \
    X(VK_MESA_extension_244) \
    X(VK_EXT_buffer_device_address) \
    X(VK_EXT_tooling_info) \
    X(VK_EXT_separate_stencil_usage) \
    X(VK_EXT_validation_features) \
    X(VK_KHR_present_wait) \
    X(VK_NV_cooperative_matrix) \
    X(VK_NV_coverage_reduction_mode) \
    X(VK_EXT_fragment_shader_interlock) \
    X(VK_EXT_ycbcr_image_arrays) \
    X(VK_KHR_uniform_buffer_standard_layout) \
    X(VK_EXT_provoking_vertex) \
    X(VK_EXT_full_screen_exclusive) \
    X(VK_EXT_headless_surface) \
    X(VK_KHR_buffer_device_address) \
    X(VK_EXT_extension_259) \
    X(VK_EXT_line_rasterization) \
    X(VK_EXT_shader_atomic_float) \
    X(VK_EXT_host_query_reset) \
    X(VK_GGP_extension_263) \
    X(VK_BRCM_extension_264) \
    X(VK_BRCM_extension_265) \
    X(VK_EXT_index_type_uint8) \
    X(VK_EXT_extension_267) \
    X(VK_EXT_extended_dynamic_state) \
    X(VK_KHR_deferred_host_operations) \
    X(VK_KHR_pipeline_executable_properties) \
    X(VK_EXT_host_image_copy) \
    X(VK_KHR_map_memory2) \
    X(VK_EXT_map_memory_placed) \
    X(VK_EXT_shader_atomic_float2) \
    X(VK_EXT_surface_maintenance1) \
    X(VK_EXT_swapchain_maintenance1) \
    X(VK_EXT_shader_demote_to_helper_invocation) \
    X(VK_NV_device_generated_commands) \
    X(VK_NV_inherited_viewport_scissor) \
    X(VK_KHR_extension_280) \
    X(VK_KHR_shader_integer_dot_product) \
    X(VK_EXT_texel_buffer_alignment) \
    X(VK_QCOM_render_pass_transform) \
    X(VK_EXT_depth_bias_control) \
    X(VK_EXT_device_memory_report) \
    X(VK_EXT_acquire_drm_display) \
    X(VK_EXT_robustness2) \
    X(VK_EXT_custom_border_color) \
    X(VK_EXT_extension_289) \
    X(VK_GOOGLE_user_type) \
    X(VK_KHR_pipeline_library) \
    X(VK_NV_extension_292) \
    X(VK_NV_present_barrier) \
    X(VK_KHR_shader_non_semantic_info) \
    X(VK_KHR_present_id) \
    X(VK_EXT_private_data) \
    X(VK_KHR_extension_297) \
    X(VK_EXT_pipeline_creation_cache_control) \
    X(VK_KHR_extension_299) \
    X(VK_KHR_video_encode_queue) \
    X(VK_NV_device_diagnostics_config) \
    X(VK_QCOM_render_pass_store_ops) \
    X(VK_QCOM_extension_303) \
    X(VK_QCOM_extension_304) \
    X(VK_QCOM_extension_305) \
    X(VK_QCOM_extension_306) \
    X(VK_QCOM_extension_307) \
    X(VK_NV_cuda_kernel_launch) \
    X(VK_KHR_object_refresh) \
    X(VK_QCOM_tile_shading) \
    X(VK_NV_low_latency) \
    X(VK_EXT_metal_objects) \
    X(VK_EXT_extension_313) \
    X(VK_AMD_extension_314) \
    X(VK_KHR_synchronization2) \
    X(VK_AMD_extension_316) \
    X(VK_EXT_descriptor_buffer) \
    X(VK_AMD_extension_318) \
    X(VK_AMD_extension_319) \
    X(VK_AMD_extension_320) \
    X(VK_EXT_graphics_pipeline_library) \
    X(VK_AMD_shader_early_and_late_fragment_tests) \
    X(VK_KHR_fragment_shader_barycentric) \
    X(VK_KHR_shader_subgroup_uniform_control_flow) \
    X(VK_KHR_extension_325) \
    X(VK_KHR_zero_initialize_workgroup_memory) \
    X(VK_NV_fragment_shading_rate_enums) \
    X(VK_NV_ray_tracing_motion_blur) \
    X(VK_EXT_mesh_shader) \
    X(VK_NV_extension_330) \
    X(VK_EXT_ycbcr_2plane_444_formats) \
    X(VK_NV_extension_332) \
    X(VK_EXT_fragment_density_map2) \
    X(VK_QCOM_rotated_copy_commands) \
    X(VK_KHR_extension_335) \
    X(VK_EXT_image_robustness) \
    X(VK_KHR_workgroup_memory_explicit_layout) \
    X(VK_KHR_copy_commands2) \
    X(VK_EXT_image_compression_control) \
    X(VK_EXT_attachment_feedback_loop_layout) \
    X(VK_EXT_4444_formats) \
    X(VK_EXT_device_fault) \
    X(VK_ARM_rasterization_order_attachment_access) \
    X(VK_ARM_extension_344) \
    X(VK_EXT_rgba10x6_formats) \
    X(VK_NV_acquire_winrt_display) \
    X(VK_EXT_directfb_surface) \
    X(VK_KHR_ray_tracing_pipeline) \
    X(VK_KHR_ray_query) \
    X(VK_KHR_extension_350) \
    X(VK_NV_extension_351) \
    X(VK_VALVE_mutable_descriptor_type) \
    X(VK_EXT_vertex_input_dynamic_state) \
    X(VK_EXT_physical_device_drm) \
    X(VK_EXT_device_address_binding_report) \
    X(VK_EXT_depth_clip_control) \
    X(VK_EXT_primitive_topology_list_restart) \
    X(VK_KHR_extension_358) \
    X(VK_EXT_extension_359) \
    X(VK_EXT_extension_360) \
    X(VK_KHR_format_feature_flags2) \
    X(VK_EXT_present_mode_fifo_latest_ready) \
    X(VK_EXT_extension_363) \
    X(VK_FUCHSIA_extension_364) \
    X(VK_FUCHSIA_external_memory) \
    X(VK_FUCHSIA_external_semaphore) \
    X(VK_FUCHSIA_buffer_collection) \
    X(VK_FUCHSIA_extension_368) \
    X(VK_QCOM_extension_369) \
    X(VK_HUAWEI_subpass_shading) \
    X(VK_HUAWEI_invocation_mask) \
    X(VK_NV_external_memory_rdma) \
    X(VK_EXT_pipeline_properties) \
    X(VK_NV_external_sci_sync) \
    X(VK_NV_external_memory_sci_buf) \
    X(VK_EXT_frame_boundary) \
    X(VK_EXT_multisampled_render_to_single_sampled) \
    X(VK_EXT_extended_dynamic_state2) \
    X(VK_QNX_screen_surface) \
    X(VK_KHR_extension_380) \
    X(VK_KHR_extension_381) \
    X(VK_EXT_color_write_enable) \
    X(VK_EXT_primitives_generated_query) \
    X(VK_EXT_extension_384) \
    X(VK_MESA_extension_385) \
    X(VK_GOOGLE_extension_386) \
    X(VK_KHR_ray_tracing_maintenance1) \
    X(VK_EXT_extension_388) \
    X(VK_EXT_global_priority_query) \
    X(VK_EXT_extension_390) \
    X(VK_EXT_extension_391) \
    X(VK_EXT_image_view_min_lod) \
    X(VK_EXT_multi_draw) \
    X(VK_EXT_image_2d_view_of_3d) \
    X(VK_KHR_portability_enumeration) \
    X(VK_EXT_shader_tile_image) \
    X(VK_EXT_opacity_micromap) \
    X(VK_NV_displacement_micromap) \
    X(VK_JUICE_extension_399) \
    X(VK_JUICE_extension_400) \
    X(VK_EXT_load_store_op_none) \
    X(VK_FB_extension_402) \
    X(VK_FB_extension_403) \
    X(VK_FB_extension_404) \
    X(VK_HUAWEI_cluster_culling_shader) \
    X(VK_HUAWEI_extension_406) \
    X(VK_GGP_extension_407) \
    X(VK_GGP_extension_408) \
    X(VK_GGP_extension_409) \
    X(VK_GGP_extension_410) \
    X(VK_GGP_extension_411) \
    X(VK_EXT_border_color_swizzle) \
    X(VK_EXT_pageable_device_local_memory) \
    X(VK_KHR_maintenance4) \
    X(VK_HUAWEI_extension_415) \
    X(VK_ARM_shader_core_properties) \
    X(VK_KHR_shader_subgroup_rotate) \
    X(VK_ARM_scheduling_controls) \
    X(VK_EXT_image_sliced_view_of_3d) \
    X(VK_EXT_extension_420) \
    X(VK_VALVE_descriptor_set_host_mapping) \
    X(VK_EXT_depth_clamp_zero_one) \
    X(VK_EXT_non_seamless_cube_map) \
    X(VK_ARM_extension_424) \
    X(VK_ARM_render_pass_striped) \
    X(VK_QCOM_fragment_density_map_offset) \
    X(VK_NV_copy_memory_indirect) \
    X(VK_NV_memory_decompression) \
    X(VK_NV_device_generated_commands_compute) \
    X(VK_NV_ray_tracing_linear_swept_spheres) \
    X(VK_NV_linear_color_attachment) \
    X(VK_NV_extension_432) \
    X(VK_NV_extension_433) \
    X(VK_GOOGLE_surfaceless_query) \
    X(VK_KHR_shader_maximal_reconvergence) \
    X(VK_EXT_application_parameters) \
    X(VK_EXT_extension_437) \
    X(VK_EXT_image_compression_control_swapchain) \
    X(VK_SEC_extension_439) \
    X(VK_QCOM_extension_440) \
    X(VK_QCOM_image_processing) \
    X(VK_COREAVI_extension_442) \
    X(VK_COREAVI_extension_443) \
    X(VK_COREAVI_extension_444) \
    X(VK_COREAVI_extension_445) \
    X(VK_COREAVI_extension_446) \
    X(VK_COREAVI_extension_447) \
    X(VK_SEC_extension_448) \
    X(VK_SEC_extension_449) \
    X(VK_SEC_extension_450) \
    X(VK_SEC_extension_451) \
    X(VK_EXT_nested_command_buffer) \
    X(VK_ARM_extension_453) \
    X(VK_EXT_external_memory_acquire_unmodified) \
    X(VK_GOOGLE_extension_455) \
    X(VK_EXT_extended_dynamic_state3) \
    X(VK_EXT_extension_457) \
    X(VK_EXT_extension_458) \
    X(VK_EXT_subpass_merge_feedback) \
    X(VK_LUNARG_direct_driver_loading) \
    X(VK_ARM_tensors) \
    X(VK_EXT_extension_462) \
    X(VK_EXT_shader_module_identifier) \
    X(VK_EXT_rasterization_order_attachment_access) \
    X(VK_NV_optical_flow) \
    X(VK_EXT_legacy_dithering) \
    X(VK_EXT_pipeline_protected_access) \
    X(VK_EXT_extension_468) \
    X(VK_ANDROID_external_format_resolve) \
    X(VK_AMD_extension_470) \
    X(VK_KHR_maintenance5) \
    X(VK_AMD_extension_472) \
    X(VK_AMD_extension_473) \
    X(VK_AMD_extension_474) \
    X(VK_AMD_extension_475) \
    X(VK_AMD_extension_476) \
    X(VK_AMD_anti_lag) \
    X(VK_AMD_extension_478) \
    X(VK_AMD_extension_479) \
    X(VK_KHR_present_id2) \
    X(VK_KHR_present_wait2) \
    X(VK_KHR_ray_tracing_position_fetch) \
    X(VK_EXT_shader_object) \
    X(VK_KHR_pipeline_binary) \
    X(VK_QCOM_tile_properties) \
    X(VK_SEC_amigo_profiling) \
    X(VK_KHR_surface_maintenance1) \
    X(VK_KHR_swapchain_maintenance1) \
    X(VK_QCOM_multiview_per_view_viewports) \
    X(VK_NV_external_sci_sync2) \
    X(VK_NV_ray_tracing_invocation_reorder) \
    X(VK_NV_cooperative_vector) \
    X(VK_NV_extended_sparse_address_space) \
    X(VK_NV_extension_494) \
    X(VK_EXT_mutable_descriptor_type) \
    X(VK_EXT_legacy_vertex_attributes) \
    X(VK_EXT_layer_settings) \
    X(VK_ARM_shader_core_builtins) \
    X(VK_EXT_pipeline_library_group_handles) \
    X(VK_EXT_dynamic_rendering_unused_attachments) \
    X(VK_EXT_extension_501) \
    X(VK_EXT_extension_502) \
    X(VK_EXT_extension_503) \
    X(VK_NV_extension_504) \
    X(VK_EXT_extension_505) \
    X(VK_NV_low_latency2) \
    X(VK_KHR_cooperative_matrix) \
    X(VK_ARM_data_graph) \
    X(VK_EXT_extension_509) \
    X(VK_MESA_extension_510) \
    X(VK_QCOM_multiview_per_view_render_areas) \
    X(VK_KHR_compute_shader_derivatives) \
    X(VK_KHR_video_decode_av1) \
    X(VK_KHR_video_encode_av1) \
    X(VK_KHR_video_decode_vp9) \
    X(VK_KHR_video_maintenance1) \
    X(VK_NV_per_stage_descriptor_set) \
    X(VK_MESA_extension_518) \
    X(VK_QCOM_image_processing2) \
    X(VK_QCOM_filter_cubic_weights) \
    X(VK_QCOM_ycbcr_degamma) \
    X(VK_QCOM_filter_cubic_clamp) \
    X(VK_EXT_extension_523) \
    X(VK_EXT_extension_524) \
    X(VK_EXT_attachment_feedback_loop_dynamic_state) \
    X(VK_KHR_vertex_attribute_divisor) \
    X(VK_KHR_load_store_op_none) \
    X(VK_KHR_unified_image_layouts) \
    X(VK_KHR_shader_float_controls2) \
    X(VK_QNX_external_memory_screen_buffer) \
    X(VK_MSFT_layered_driver) \
    X(VK_KHR_extension_532) \
    X(VK_EXT_extension_533) \
    X(VK_KHR_index_type_uint8) \
    X(VK_KHR_line_rasterization) \
    X(VK_QCOM_extension_536) \
    X(VK_EXT_extension_537) \
    X(VK_EXT_extension_538) \
    X(VK_EXT_extension_539) \
    X(VK_EXT_extension_540) \
    X(VK_EXT_extension_541) \
    X(VK_EXT_extension_542) \
    X(VK_EXT_extension_543) \
    X(VK_KHR_calibrated_timestamps) \
    X(VK_KHR_shader_expect_assume) \
    X(VK_KHR_maintenance6) \
    X(VK_NV_descriptor_pool_overallocation) \
    X(VK_QCOM_tile_memory_heap) \
    X(VK_NV_extension_549) \
    X(VK_NV_extension_550) \
    X(VK_NV_extension_551) \
    X(VK_NV_display_stereo) \
    X(VK_KHR_video_encode_intra_refresh) \
    X(VK_KHR_video_encode_quantization_map) \
    X(VK_IMG_extension_555) \
    X(VK_NV_raw_access_chains) \
    X(VK_NV_external_compute_queue) \
    X(VK_KHR_extension_558) \
    X(VK_KHR_shader_relaxed_extended_instruction) \
    X(VK_NV_command_buffer_inheritance) \
    X(VK_EXT_extension_561) \
    X(VK_KHR_extension_562) \
    X(VK_KHR_maintenance7) \
    X(VK_NV_shader_atomic_float16_vector) \
    X(VK_EXT_shader_replicated_composites) \
    X(VK_ARM_extension_566) \
    X(VK_ARM_extension_567) \
    X(VK_EXT_shader_float8) \
    X(VK_NV_ray_tracing_validation) \
    X(VK_NV_cluster_acceleration_structure) \
    X(VK_NV_partitioned_acceleration_structure) \
    X(VK_NV_extension_572) \
    X(VK_EXT_device_generated_commands) \
    X(VK_KHR_extension_574) \
    X(VK_KHR_maintenance8) \
    X(VK_MESA_image_alignment_control) \
    X(VK_HUAWEI_extension_577) \
    X(VK_EXT_extension_578) \
    X(VK_EXT_extension_579) \
    X(VK_EXT_extension_580) \
    X(VK_NV_extension_581) \
    X(VK_EXT_extension_582) \
    X(VK_EXT_depth_clamp_control) \
    X(VK_EXT_extension_584) \
    X(VK_KHR_maintenance9) \
    X(VK_IMG_extension_586) \
    X(VK_KHR_video_maintenance2) \
    X(VK_OHOS_surface) \
    X(VK_HUAWEI_extension_589) \
    X(VK_HUAWEI_extension_590) \
    X(VK_HUAWEI_hdr_vivid) \
    X(VK_NV_extension_592) \
    X(VK_NV_extension_593) \
    X(VK_NV_cooperative_matrix2) \
    X(VK_NV_extension_595) \
    X(VK_KHR_extension_596) \
    X(VK_ARM_pipeline_opacity_micromap) \
    X(VK_KHR_extension_598) \
    X(VK_KHR_extension_599) \
    X(VK_IMG_extension_600) \
    X(VK_IMG_extension_601) \
    X(VK_EXT_extension_602) \
    X(VK_EXT_external_memory_metal) \
    X(VK_EXT_extension_604) \
    X(VK_KHR_depth_clamp_zero_one) \
    X(VK_KHR_extension_606) \
    X(VK_KHR_extension_607) \
    X(VK_KHR_extension_608) \
    X(VK_EXT_vertex_attribute_robustness) \
    X(VK_ARM_format_pack) \
    X(VK_NV_extension_611) \
    X(VK_VALVE_fragment_density_map_layered) \
    X(VK_KHR_robustness2) \
    X(VK_NV_present_metering) \
    X(VK_QCOM_extension_615) \
    X(VK_EXT_extension_616) \
    X(VK_EXT_extension_617) \
    X(VK_EXT_extension_618) \
    X(missing_extension_number_619) \
    X(VK_EXT_fragment_density_map_offset) \
    X(VK_EXT_zero_initialize_device_memory) \
    X(VK_KHR_present_mode_fifo_latest_ready) \
    X(VK_EXT_extension_623) \
    X(VK_KHR_extension_624) \
    X(VK_KHR_extension_625) \
    X(VK_EXT_extension_626) \
    X(VK_NV_extension_627) \
    X(VK_EXT_extension_628) \
    X(VK_EXT_extension_629) \
    X(VK_EXT_extension_630) \
    X(VK_KHR_extension_631) \
    X(VK_ARM_extension_632) \
    X(VK_MTK_extension_633) \
    X(VK_NV_extension_634) \
    X(VK_MTK_extension_635) \
    X(VK_EXT_extension_636) \
    X(VK_EXT_extension_637) \
    X(VK_SEC_pipeline_cache_incremental_mode)

namespace ExtensionIDs {
    // X(name, id) name = id,
#define X(id) id,
    enum ExtensionIDs : uint16_t {
        NONE = 0,
        EXTENSION_LIST(X)
        _COUNT
    };
#undef X
    // X(name, id) [id] = #name,
#define X(id) [id] = #id,
    static const char* const names[] = { [NONE] = "NONE", EXTENSION_LIST(X) };
#undef X
    inline const char* to_cstr(ExtensionIDs id) { if (id >= ExtensionIDs::_COUNT) return "Invalid"; return names[id]; }
    constexpr auto a = VK_KHR_ray_tracing_pipeline;
}


enum class ToStringFunction : uint8_t {
    None,  // Don't generate to_cstr()
    InCpp, // Generate declarations in hpp but define it in cpp
    InHpp, // Generate inline to_cstr() in hpp
};

enum class STLClassesInfo : uint8_t {
    UseStdImpl, // include <vector> and use std::vector/...
    UseOwnImpl, // use own implementation
    DetectIfImplNeeded // Dont include <vector> but use it if its already included, also generate own implementation for fall back
};

enum class ExceptionBehavior : uint8_t {
    ThrowOnly,              // Generate only throwing functions, without suffix
    NoThrowOnly,            // Generate only nothrow functions, without suffix
    BothWithDefaultThrow,   // Generate throw and nothrow functions with suffixes, generate additional default function that calls throw function
    BothWithDefaultNoThrow, // Generate throw and nothrow functions with suffixes, generate additional default function that calls nothrow function
    BothWithoutDefault      // Generate throw and nothrow functions with suffixes
};

enum class BetaExtensions : uint8_t {
    DontGenerate,
    GenerateWithProtectMacro,
    Generate,
};

enum class ToCstrFunction : uint8_t {
    None,  // Don't generate to_cstr()
    InCpp, // Generate declarations in hpp but define it in cpp
    InHpp, // Generate inline to_cstr() in hpp
};

struct Config {
    std::vector<bool> enabled_extensions = std::vector<bool>((size_t)ExtensionIDs::_COUNT, false); // use bitset

    bool generate_comments = true;
    bool generate_deprecations = true;
    bool generate_enums_classes = true; // false means generate C enums
    bool generate_handle_class = true;  // false means generate C vulkan macros
    bool generate_flags_class = true;   // false means generate C VkFlags (uint32) and VkFlags64 (uint64)
    bool generate_enum_numbers = true;  // false tries to remove all unnecessary enum numbers
    bool generate_c_type_keywords = true; // C requires (struct|union|enum) before type name, C++ doesn't
    bool apply_av1_and_vp9_naming_exceptions = true; // AV1 and VP9 would be translated to "Av1" and "Vp9" in C++
    bool generate_command_aliases = true; // false means skipping aliases for commands, taht usually only adds extension suffix

    Compact compact = Compact::Normal;
    LogLevel log_level = LogLevel::Warning;
    DeprecationBehavior deprecation_behavior = DeprecationBehavior::GenerateWithDeprecationWarning;
    BetaExtensions beta_extensions = BetaExtensions::GenerateWithProtectMacro;
    ExceptionBehavior exception_behavior = ExceptionBehavior::BothWithDefaultThrow;
    ToCstrFunction to_cstr_behavior = ToCstrFunction::None;

    // C++ features: namespacing?
    // C++ features: modules?

    bool generate_extension_defined_macro = false;
    bool generate_extension_name_macro = false;
    //bool protect_extension = false;

    bool header_only = false;
    const char* header_only_guard = "VKG_IMPLEMENTATION";

    const char* namespace_name = "vk"; // nullptr means no namespace
    const char* header_path = "out.hpp";
    const char* source_path = "out.cpp";
    const char* xml_path = "vk.xml";

    ToStringFunction to_string_function = ToStringFunction::None;
    STLClassesInfo vector = STLClassesInfo::UseStdImpl; // use std::vector or own implementation?
    STLClassesInfo iterator = STLClassesInfo::UseStdImpl;
    STLClassesInfo string_view = STLClassesInfo::UseStdImpl;

};

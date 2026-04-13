#include "player.h"
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/input.hpp>
#include <godot_cpp/classes/animated_sprite2d.hpp>
#include <godot_cpp/classes/sprite_frames.hpp>
#include <godot_cpp/classes/area2d.hpp>
#include <godot_cpp/classes/collision_shape2d.hpp>
#include <godot_cpp/classes/rectangle_shape2d.hpp>
#include <godot_cpp/classes/character_body2d.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/variant/callable.hpp>

namespace godot {

Player::Player() : 
    walk_speed(200.0f),
    run_speed(400.0f),
    jump_force(400.0f),
    forward_jump_force(350.0f),
    up_jump_force(450.0f),
    roll_speed(500.0f),
    roll_duration(0.9f),
    roll_timer(0.0f),
    gravity(980.0f),
    is_crouching(false),       // 下蹲状态
    crouch_timer(0.0f),        // 下蹲计时器
    crouch_duration(0.3f),     // 下蹲动画持续时间（0.3秒）
    is_rolling(false),
    is_dead(false),
    is_running(false),
    is_sprite_flipped_h(false),
    is_invincible(false),      // 无敌状态
    invincible_timer(0.0f),    // 无敌计时器
    invincible_duration(0.3f), // 无敌持续时间（0.3秒）
    current_attack_type(NO_ATTACK),  // 攻击状态
    attack_timer(0.0f),
    attack_duration(0.4f),     // 攻击动画持续时间
    is_attacking(false),
    attack_damage(10.0f),      // 基础伤害
    attack_knockback(Vector2(200.0f, -50.0f)), // 击退力
    max_health(100.0f),          // 最大生命值
    current_health(100.0f),      // 当前生命值
    health_regen_rate(0.0f),     // 生命恢复速率（0表示不自动恢复）
    health_regen_timer(0.0f),    // 生命恢复计时器
    current_jump_type(NO_JUMP),
    is_jumping(false),
    is_landing(false),
    landing_timer(0.0f),
    input_direction(0.0f),
    vertical_input(0.0f),      // 保留但不再使用
    jump_pressed(false),
    run_pressed(false),
    crouch_pressed(false),
    crouch_was_pressed(false),
    roll_pressed(false),
    attack_1_pressed(false),   // 攻击1
    attack_2_pressed(false),   // 攻击2
    player_id(1),              // 默认玩家1
    animated_sprite(nullptr),
    attack_area(nullptr),
    hurtbox_area(nullptr),       // 引用手动创建的HurtBox节点
    attack_range(50.0f),        // 攻击范围
    attack_width(30.0f)         // 攻击宽度
{}

Player::~Player() {
    // 清理资源
}

void Player::_bind_methods() {
    // 基础属性
    ClassDB::bind_method(D_METHOD("get_walk_speed"), &Player::get_walk_speed);
    ClassDB::bind_method(D_METHOD("set_walk_speed", "speed"), &Player::set_walk_speed);
    ClassDB::bind_method(D_METHOD("get_run_speed"), &Player::get_run_speed);
    ClassDB::bind_method(D_METHOD("set_run_speed", "speed"), &Player::set_run_speed);
    ClassDB::bind_method(D_METHOD("get_jump_force"), &Player::get_jump_force);
    ClassDB::bind_method(D_METHOD("set_jump_force", "force"), &Player::set_jump_force);
    ClassDB::bind_method(D_METHOD("get_forward_jump_force"), &Player::get_forward_jump_force);
    ClassDB::bind_method(D_METHOD("set_forward_jump_force", "force"), &Player::set_forward_jump_force);
    ClassDB::bind_method(D_METHOD("get_up_jump_force"), &Player::get_up_jump_force);
    ClassDB::bind_method(D_METHOD("set_up_jump_force", "force"), &Player::set_up_jump_force);
    ClassDB::bind_method(D_METHOD("get_gravity"), &Player::get_gravity);
    ClassDB::bind_method(D_METHOD("set_gravity", "value"), &Player::set_gravity);
    
    // 动画控制
    ClassDB::bind_method(D_METHOD("play_animation", "anim_name", "force_restart"), &Player::play_animation, DEFVAL(false));
    ClassDB::bind_method(D_METHOD("get_animated_sprite_path"), &Player::get_animated_sprite_path);
    ClassDB::bind_method(D_METHOD("set_animated_sprite_path", "path"), &Player::set_animated_sprite_path);
    
    // 攻击属性
    ClassDB::bind_method(D_METHOD("get_attack_damage"), &Player::get_attack_damage);
    ClassDB::bind_method(D_METHOD("set_attack_damage", "damage"), &Player::set_attack_damage);
    ClassDB::bind_method(D_METHOD("get_attack_range"), &Player::get_attack_range);
    ClassDB::bind_method(D_METHOD("set_attack_range", "range"), &Player::set_attack_range);
    ClassDB::bind_method(D_METHOD("get_attack_width"), &Player::get_attack_width);
    ClassDB::bind_method(D_METHOD("set_attack_width", "width"), &Player::set_attack_width);
    ClassDB::bind_method(D_METHOD("get_attack_knockback"), &Player::get_attack_knockback);
    ClassDB::bind_method(D_METHOD("set_attack_knockback", "knockback"), &Player::set_attack_knockback);
    
    // 无敌帧属性
    ClassDB::bind_method(D_METHOD("get_invincible_duration"), &Player::get_invincible_duration);
    ClassDB::bind_method(D_METHOD("set_invincible_duration", "duration"), &Player::set_invincible_duration);
    ClassDB::bind_method(D_METHOD("get_is_invincible"), &Player::get_is_invincible);
    
    // 玩家标识
    ClassDB::bind_method(D_METHOD("get_player_id"), &Player::get_player_id);
    ClassDB::bind_method(D_METHOD("set_player_id", "id"), &Player::set_player_id);
    
    // 血量属性绑定
    ClassDB::bind_method(D_METHOD("take_damage", "damage"), &Player::take_damage);
    ClassDB::bind_method(D_METHOD("heal", "amount"), &Player::heal);
    ClassDB::bind_method(D_METHOD("reset_health"), &Player::reset_health);
    ClassDB::bind_method(D_METHOD("get_max_health"), &Player::get_max_health);
    ClassDB::bind_method(D_METHOD("set_max_health", "health"), &Player::set_max_health);
    ClassDB::bind_method(D_METHOD("get_current_health"), &Player::get_current_health);
    ClassDB::bind_method(D_METHOD("set_current_health", "health"), &Player::set_current_health);
    ClassDB::bind_method(D_METHOD("get_health_percentage"), &Player::get_health_percentage);
    
    // 状态查询
    ClassDB::bind_method(D_METHOD("get_is_crouching"), &Player::get_is_crouching);
    ClassDB::bind_method(D_METHOD("get_is_running"), &Player::get_is_running);
    ClassDB::bind_method(D_METHOD("get_is_dead"), &Player::get_is_dead);
    ClassDB::bind_method(D_METHOD("get_is_jumping"), &Player::get_is_jumping);
    ClassDB::bind_method(D_METHOD("get_is_landing"), &Player::get_is_landing);
    ClassDB::bind_method(D_METHOD("get_current_jump_type"), &Player::get_current_jump_type);
    ClassDB::bind_method(D_METHOD("get_current_attack_type"), &Player::get_current_attack_type);
    ClassDB::bind_method(D_METHOD("get_is_attacking"), &Player::get_is_attacking);
    
    // 区域节点访问
    ClassDB::bind_method(D_METHOD("get_attack_area"), &Player::get_attack_area);
    ClassDB::bind_method(D_METHOD("get_hurtbox_area"), &Player::get_hurtbox_area);
    
    // 编辑器属性
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "walk_speed"), "set_walk_speed", "get_walk_speed");
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "run_speed"), "set_run_speed", "get_run_speed");
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "jump_force"), "set_jump_force", "get_jump_force");
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "forward_jump_force"), "set_forward_jump_force", "get_forward_jump_force");
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "up_jump_force"), "set_up_jump_force", "get_up_jump_force");
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "gravity"), "set_gravity", "get_gravity");
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "max_health"), "set_max_health", "get_max_health");
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "current_health"), "set_current_health", "get_current_health");
    
    // 攻击属性
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "attack_damage"), "set_attack_damage", "get_attack_damage");
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "attack_range"), "set_attack_range", "get_attack_range");
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "attack_width"), "set_attack_width", "get_attack_width");
    ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "attack_knockback"), "set_attack_knockback", "get_attack_knockback");
    
    // 无敌帧属性
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "invincible_duration"), "set_invincible_duration", "get_invincible_duration");
    
    // 玩家标识
    ADD_PROPERTY(PropertyInfo(Variant::INT, "player_id"), "set_player_id", "get_player_id");
    
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "animated_sprite_path", PROPERTY_HINT_NODE_PATH_TO_EDITED_NODE, "AnimatedSprite2D"), 
                  "set_animated_sprite_path", "get_animated_sprite_path");
}

void Player::_ready() {
    if (Engine::get_singleton()->is_editor_hint()) {
        return;
    }
    
    set_velocity(Vector2(0, 0));
    
    // 获取动画精灵
    animated_sprite = get_node<AnimatedSprite2D>(NodePath("AnimatedSprite2D"));
    if (!animated_sprite) {
        UtilityFunctions::printerr("Player: 未能找到 AnimatedSprite2D 子节点");
    } else {
        play_animation("idle");
    }
    
    // 获取手动创建的AttackArea（攻击检测区域）
    attack_area = get_node<Area2D>(NodePath("AttackArea"));
    if (!attack_area) {
        UtilityFunctions::printerr("Player: 未能找到手动创建的AttackArea节点。请确保在场景中为该Player添加了名为'AttackArea'的Area2D子节点。");
    } else {
        // 初始时禁用攻击检测
        disable_attack_detection();
        
        // 更新攻击区域位置
        update_attack_area_position();
    }
    
    // 获取手动创建的HurtBox（受击区域）
    hurtbox_area = get_node<Area2D>(NodePath("HurtBox"));
    if (!hurtbox_area) {
        UtilityFunctions::printerr("Player: 未能找到手动创建的HurtBox节点。请确保在场景中为该Player添加了名为'HurtBox'的Area2D子节点。");
    }
    
    UtilityFunctions::print(vformat("玩家 %d 初始化完成", player_id));
}

void Player::_physics_process(double delta) {
    if (Engine::get_singleton()->is_editor_hint() || is_dead) {
        return;
    }
    
    // 1. 处理输入
    handle_input();
    
    // 2. 记录落地前的状态
    bool was_on_floor = is_on_floor();
    
    // 3. 应用移动
    apply_movement(delta);
    
    // 4. 应用物理
    move_and_slide();
    
    // 5. 检测着陆
    if (is_on_floor() && !was_on_floor && is_jumping) {
        start_landing();
    }
    
    // 6. 更新着陆计时器
    if (is_landing) {
        landing_timer -= delta;
        if (landing_timer <= 0) {
            is_landing = false;
            current_jump_type = NO_JUMP;
        }
    }
    
    // 7. 处理下蹲
    handle_crouch(delta);
    
    // 8. 处理攻击
    handle_attack(delta);
    
    // 9. 更新攻击检测区域位置
    if (attack_area) {
        update_attack_area_position();
    }
    
    // 10. 处理命中检测
    if (is_attacking) {
        process_hit_detection();
    }
    
    // 11. 更新状态
    update_state();
    
    // 12. 更新动画
    update_animation();
    
    // 13. 更新翻滚计时
    if (is_rolling) {
        roll_timer -= delta;
        if (roll_timer <= 0) {
            is_rolling = false;
            // 翻滚结束时，启动无敌帧
            enable_invincibility();
        }
    }
    
    // 14. 记录上一帧的crouch_pressed状态
    crouch_was_pressed = crouch_pressed;

    // 15. 处理生命恢复
    if (health_regen_rate > 0.0f && current_health < max_health && !is_dead) {
        health_regen_timer += delta;
        if (health_regen_timer >= 1.0f) {  // 每秒恢复一次
            heal(health_regen_rate);
            health_regen_timer = 0.0f;
        }
    }
    
    // 16. 更新无敌帧
    update_invincibility(delta);
    
    // 调试输出
    static int frame_count = 0;
    frame_count++;
    if (frame_count % 60 == 0) {
        UtilityFunctions::print(vformat(
            "玩家%d: 地面=%s, 攻击=%s(%d), 翻滚=%s, 无敌=%s(%.2f), 血量=%.1f/%.1f", 
            player_id,
            is_on_floor() ? "是" : "否",
            is_attacking ? "是" : "否",
            (int)current_attack_type,
            is_rolling ? "是" : "否",
            is_invincible ? "是" : "否",
            invincible_timer,
            current_health,
            max_health
        ));
    }
}

// 无敌帧控制方法
void Player::enable_invincibility() {
    is_invincible = true;
    invincible_timer = invincible_duration;
    
    if (hurtbox_area) {
        // 禁用HurtBox的碰撞检测
        hurtbox_area->set_collision_layer(0);
        hurtbox_area->set_collision_mask(0);
    }
    
    // 更新HurtBox可见性（闪烁效果）
    update_hurtbox_visibility();
    
    UtilityFunctions::print(vformat("玩家%d进入无敌状态", player_id));
}

void Player::disable_invincibility() {
    is_invincible = false;
    invincible_timer = 0.0f;
    
    if (hurtbox_area) {
        // 重新启用HurtBox的碰撞检测
        // 假设HurtBox在第2层，AttackArea检测第2层
        hurtbox_area->set_collision_layer(2);
        hurtbox_area->set_collision_mask(0);
        
        // 确保HurtBox可见
        hurtbox_area->set_visible(true);
    }
    
    UtilityFunctions::print(vformat("玩家%d离开无敌状态", player_id));
}

void Player::update_invincibility(double delta) {
    if (is_invincible) {
        invincible_timer -= delta;
        
        // 更新HurtBox闪烁效果
        update_hurtbox_visibility();
        
        if (invincible_timer <= 0) {
            disable_invincibility();
        }
    }
}

void Player::update_hurtbox_visibility() {
    if (!hurtbox_area) return;
    
    // 简单的闪烁效果：每0.1秒切换一次可见性
    static float blink_timer = 0.0f;
    blink_timer += 0.016f; // 假设60FPS，每帧0.016秒
    
    if (blink_timer >= 0.1f) { // 每0.1秒闪烁一次
        blink_timer = 0.0f;
        bool is_visible = hurtbox_area->is_visible();
        hurtbox_area->set_visible(!is_visible);
    }
}

void Player::handle_input() {
    Input *input = Input::get_singleton();
    
    // 构建动作名称后缀
    String suffix = (player_id == 1) ? "_p1" : "_p2";
    
    // 检查连接的手柄数量
    Array connected_joypads = input->get_connected_joypads();
    int connected_count = connected_joypads.size();
    
    // 调试：打印连接的手柄
    static bool printed_joypads = false;
    if (!printed_joypads) {
        UtilityFunctions::print(vformat("连接的手柄数量: %d", connected_count));
        for (int i = 0; i < connected_count; i++) {
            UtilityFunctions::print(vformat("  手柄%d: 设备索引 %d", i, (int)connected_joypads[i]));
        }
        printed_joypads = true;
    }
    
    // 如果只有一个手柄连接，玩家2使用键盘
    if (player_id == 2 && connected_count < 2) {
        // 玩家2使用键盘（临时方案）
        handle_input_keyboard();
    } else {
        // 如果手柄数量足够，使用手柄输入
        handle_input_gamepad(suffix);
    }
}

// 手柄输入处理
void Player::handle_input_gamepad(const String& suffix) {
    Input *input = Input::get_singleton();
    
    // 水平移动
    input_direction = 0.0f;
    if (input->is_action_pressed("move_left" + suffix)) {
        input_direction -= 1.0f;
        // UtilityFunctions::print(vformat("玩家%d 向左移动 (手柄)", player_id));
    }
    if (input->is_action_pressed("move_right" + suffix)) {
        input_direction += 1.0f;
        // UtilityFunctions::print(vformat("玩家%d 向右移动 (手柄)", player_id));
    }
    
    // 垂直移动
    vertical_input = 0.0f;
    if (input->is_action_pressed("move_up" + suffix)) vertical_input += 1.0f;
    if (input->is_action_pressed("move_down" + suffix)) vertical_input -= 1.0f;
    
    // 动作按键
    jump_pressed = input->is_action_just_pressed("jump" + suffix);
    if (jump_pressed) {
        UtilityFunctions::print(vformat("玩家%d 按下跳跃 (手柄)", player_id));
    }
    
    run_pressed = input->is_action_pressed("run" + suffix);
    crouch_pressed = input->is_action_pressed("crouch" + suffix);
    roll_pressed = input->is_action_just_pressed("roll" + suffix);
    
    // 攻击按键
    attack_1_pressed = input->is_action_just_pressed("attack_1" + suffix);
    attack_2_pressed = input->is_action_just_pressed("attack_2" + suffix);
}

// 键盘输入处理（备用方案）
void Player::handle_input_keyboard() {
    Input *input = Input::get_singleton();
    
    // 玩家2使用键盘控制
    if (player_id == 2) {
        // 水平移动 - 方向键
        input_direction = 0.0f;
        if (input->is_action_pressed("ui_left")) {
            input_direction -= 1.0f;   // 左箭头
            // UtilityFunctions::print("玩家2 向左移动 (键盘)");
        }
        if (input->is_action_pressed("ui_right")) {
            input_direction += 1.0f;  // 右箭头
            // UtilityFunctions::print("玩家2 向右移动 (键盘)");
        }
        
        // 垂直移动
        vertical_input = 0.0f;
        if (input->is_action_pressed("ui_up")) vertical_input += 1.0f;     // 上箭头
        if (input->is_action_pressed("ui_down")) vertical_input -= 1.0f;   // 下箭头
        
        // 动作按键
        jump_pressed = input->is_action_just_pressed("ui_select");   // 回车键
        if (jump_pressed) {
            UtilityFunctions::print("玩家2 按下跳跃 (键盘)");
        }
        
        run_pressed = input->is_action_pressed("ui_accept");         // Shift键
        crouch_pressed = input->is_action_pressed("ui_ctrl");        // Ctrl键
        roll_pressed = input->is_action_just_pressed("ui_home");     // Home键
        
        // 攻击按键
        attack_1_pressed = input->is_action_just_pressed("ui_end");     // End键
        attack_2_pressed = input->is_action_just_pressed("ui_pageup");  // PageUp键
    }
}

void Player::apply_movement(double delta) {
    if (is_dead) {
        set_velocity(Vector2(0, 0));
        return;
    }
    
    Vector2 velocity = get_velocity();
    
    // 应用重力
    if (!is_dead) {
        velocity.y += gravity * delta;
    }
    
    if (is_rolling) {
        // 翻滚移动
        velocity.x = input_direction * roll_speed;
    } else {
        // 根据是否在地面，选择不同的速度策略
        if (is_on_floor()) {
            // 地面移动
            // 攻击或下蹲状态下不允许移动
            if (!is_attacking && !is_crouching) {
                float target_speed = walk_speed;
                if (is_running) target_speed = run_speed;
                
                velocity.x = input_direction * target_speed;
            } else {
                // 攻击或下蹲状态下，水平速度设为0
                velocity.x = 0;
            }
        } else {
            // 空中移动
            if (is_jumping) {
                // 跳跃中，根据跳跃类型设置水平速度
                if (current_jump_type == FORWARD_JUMP) {
                    // forward_jump：空中速度为running_speed
                    velocity.x = input_direction * run_speed;
                } else if (current_jump_type == UP_JUMP) {
                    // up_jump：空中速度为walk_speed
                    velocity.x = input_direction * walk_speed;
                } else {
                    // 其他跳跃情况，使用walk_speed
                    velocity.x = input_direction * walk_speed;
                }
            } else {
                // 非跳跃的空中状态（如掉落），使用walk_speed
                velocity.x = input_direction * walk_speed;
            }
        }
    }
    
    set_velocity(velocity);
}

void Player::handle_crouch(double delta) {
    // 更新下蹲计时器
    if (crouch_timer > 0) {
        crouch_timer -= delta;
    }
    
    // 检测按键按下
    if (crouch_pressed && !crouch_was_pressed && is_on_floor()) {
        if (!is_crouching && !is_attacking) {  // 攻击时不能下蹲
            // 开始下蹲
            is_crouching = true;
            crouch_timer = crouch_duration;
            UtilityFunctions::print(vformat("玩家%d开始下蹲", player_id));
        }
    }
    
    // 检测按键松开
    if (!crouch_pressed && crouch_was_pressed) {
        if (is_crouching && crouch_timer <= 0) {
            // 开始起身，设置计时器但不改变is_crouching状态
            crouch_timer = crouch_duration;
            UtilityFunctions::print(vformat("玩家%d开始起身，设置计时器", player_id));
        }
    }
}

void Player::handle_attack(double delta) {
    // 更新攻击计时器
    if (attack_timer > 0) {
        attack_timer -= delta;
        if (attack_timer <= 0) {
            // 攻击动画播放完毕
            end_attack();
        }
    }
    
    // 检测攻击1按下（原拳击1）
    if (attack_1_pressed && !is_attacking && !is_crouching) {
        start_attack(PUNCH_1);
    }
    
    // 检测攻击2按下（原拳击2）
    if (attack_2_pressed && !is_attacking && !is_crouching) {
        start_attack(PUNCH_2);
    }
}

void Player::start_attack(AttackType attack_type) {
    if (is_attacking) return;  // 已经在攻击中，不重复开始
    
    is_attacking = true;
    current_attack_type = attack_type;
    attack_timer = attack_duration;
    
    // 重置命中目标列表
    reset_hit_targets();
    
    // 启用攻击检测
    enable_attack_detection();
    
    UtilityFunctions::print(vformat("玩家%d开始攻击: %s", player_id, attack_type == PUNCH_1 ? "左拳" : "右拳"));
}

void Player::end_attack() {
    is_attacking = false;
    current_attack_type = NO_ATTACK;
    attack_timer = 0.0f;
    
    // 禁用攻击检测
    disable_attack_detection();
    
    // 重置命中目标列表
    reset_hit_targets();
    
    UtilityFunctions::print(vformat("玩家%d攻击结束", player_id));
}

void Player::update_attack_area_position() {
    if (!attack_area) return;
    
    // 固定的攻击范围偏移量
    float attack_offset = 21.0f;  // 可以根据需要调整
    
    // 根据角色朝向设置攻击区域位置
    if (is_sprite_flipped_h) {
        // 朝左时，攻击区域在左侧
        attack_area->set_position(Vector2(-attack_offset, 0));
    } else {
        // 朝右时，攻击区域在右侧
        attack_area->set_position(Vector2(attack_offset, 0));
    }
}

void Player::enable_attack_detection() {
    if (attack_area) {
        // 通过启用Area2D的监控来启用攻击检测
        attack_area->set_monitoring(true);
        attack_area->set_monitorable(true);
    }
}

void Player::disable_attack_detection() {
    if (attack_area) {
        // 通过禁用Area2D的监控来禁用攻击检测
        attack_area->set_monitoring(false);
        attack_area->set_monitorable(false);
    }
}

void Player::process_hit_detection() {
    if (!attack_area || !is_attacking) return;
    
    // 获取攻击区域内的所有重叠的区域（HurtBox）
    Array overlapping_areas = attack_area->get_overlapping_areas();
    
    for (int i = 0; i < overlapping_areas.size(); i++) {
        // 从Variant获取Area2D指针
        Variant area_variant = overlapping_areas[i];
        Area2D* area = Object::cast_to<Area2D>(area_variant);
        if (!area) continue;
        
        // 跳过自己的HurtBox
        if (area == hurtbox_area) continue;
        
        // 获取HurtBox的所有者（应该是另一个Player）
        Node* owner = area->get_parent();
        if (!owner) continue;
        
        // 检查是否已经命中过这个目标
        bool already_hit = false;
        for (int j = 0; j < hit_targets.size(); j++) {
            // 将Node*转换为Variant进行比较
            if (hit_targets[j] == Variant(owner)) {
                already_hit = true;
                break;
            }
        }
        
        if (!already_hit) {
            // 处理命中
            UtilityFunctions::print(vformat("玩家%d命中目标: %s", player_id, owner->get_name()));
            
            // 添加目标到已命中列表
            hit_targets.append(Variant(owner));
            
            // 应用击退效果
            apply_attack_knockback(owner);

            // 对目标造成伤害
            Player* target_player = Object::cast_to<Player>(owner);
            if (target_player) {
                target_player->take_damage(attack_damage);
            } else {
                // 如果不是Player类型，尝试调用目标的take_damage方法
                if (owner->has_method("take_damage")) {
                    owner->call("take_damage", attack_damage);
                }
            }
        }
    }
}

void Player::reset_hit_targets() {
    hit_targets.clear();
}

void Player::apply_attack_knockback(Node* target) {
    if (!target) return;
    
    // 检查目标是否是CharacterBody2D
    CharacterBody2D* character = Object::cast_to<CharacterBody2D>(target);
    if (!character) return;
    
    // 计算击退方向
    Vector2 knockback_direction = Vector2(attack_knockback.x, attack_knockback.y);
    
    // 根据攻击方向调整水平击退
    if (is_sprite_flipped_h) {
        knockback_direction.x = -knockback_direction.x;
    }
    
    // 应用击退速度
    Vector2 velocity = character->get_velocity();
    velocity += knockback_direction;
    character->set_velocity(velocity);
    
    UtilityFunctions::print(vformat("玩家%d对目标应用击退: %s", player_id, target->get_name()));
}

void Player::update_state() {
    // 死亡状态
    if (is_dead) {
        play_animation("falling");
        return;
    }
    
    // 着陆状态
    if (is_landing) {
        play_animation("landing");
        return;
    }
    
    // 攻击状态（高优先级）
    if (is_attacking) {
        // 根据攻击类型播放相应动画
        if (current_attack_type == PUNCH_1) {
            play_animation("punch_1");
        } else if (current_attack_type == PUNCH_2) {
            play_animation("punch_2");
        }
        return;
    }
    
    // 翻滚状态
    if (is_rolling) {
        play_animation("roll");
        return;
    }
    
    // 处理跳跃触发
    if (jump_pressed && is_on_floor() && !is_jumping && !is_landing && !is_crouching && !is_attacking) {
        UtilityFunctions::print(vformat("玩家%d检测到跳跃按键，当前奔跑状态: %s", player_id, is_running ? "是" : "否"));
        if (is_running) {
            start_jump(FORWARD_JUMP);
        } else {
            start_jump(UP_JUMP);
        }
        return;
    }
    
    // 空中状态
    if (!is_on_floor()) {
        if (is_jumping) {
            // 跳跃中，根据跳跃类型播放对应的跳跃动画
            if (current_jump_type == FORWARD_JUMP) {
                play_animation("forward_jump", false);
            } else if (current_jump_type == UP_JUMP) {
                play_animation("up_jump", false);
            } else {
                // 意外的跳跃状态，回退到up_jump动画
                play_animation("up_jump");
            }
        } else {
            // 非跳跃的空中状态（如掉落）
            play_animation("up_jump");
        }
        return;
    }
    
    // 地面状态
    if (is_crouching) {
        // 下蹲状态
        if (crouch_timer > 0) {
            // 正在播放下蹲/起身动画
            if (crouch_pressed) {
                // 按下状态，正在下蹲
                play_animation("crouch_down");
            } else {
                // 松开状态，正在起身
                play_animation("crouch_up");
            }
        } else {
            // 计时器结束
            if (crouch_pressed) {
                // 按键还按着，保持下蹲状态
                play_animation("crouch_stay");
            } else {
                // 按键已松开，结束下蹲状态
                is_crouching = false;
                UtilityFunctions::print(vformat("玩家%d起身动画播放完毕，结束下蹲状态", player_id));
            }
        }
        return;
    } else if (is_running) {
        if (input_direction != 0) {
            play_animation("running");
        } else {
            play_animation("stop_running");
        }
    } else if (input_direction != 0) {
        play_animation("walk");
    } else {
        play_animation("idle");
    }
    
    // 翻滚触发
    if (roll_pressed && is_on_floor() && !is_rolling && !is_crouching && !is_attacking) {
        is_rolling = true;
        roll_timer = roll_duration;
        // 翻滚开始时立即启用无敌状态
        enable_invincibility();
    }
    
    // 奔跑状态更新
    if (run_pressed && !is_running && !is_crouching && !is_attacking) {
        is_running = true;
    } else if (!run_pressed && is_running) {
        is_running = false;
    }
}

void Player::update_animation() {
    if (!animated_sprite) return;
    
    // 更新精灵朝向
    if (input_direction != 0) {
        bool should_flip = input_direction < 0;
        if (should_flip != is_sprite_flipped_h) {
            animated_sprite->set_flip_h(should_flip);
            is_sprite_flipped_h = should_flip;
        }
    }
}

void Player::play_animation(const String& anim_name, bool force_restart) {
    if (!animated_sprite) return;
    
    // 特别处理crouch_down动画，使用try-catch避免错误
    if (anim_name == "crouch_down") {
        try {
            if (force_restart || animated_sprite->get_animation() != StringName(anim_name)) {
                animated_sprite->play(anim_name);
                UtilityFunctions::print(vformat("玩家%d播放crouch_down动画", player_id));
            }
        } catch (...) {
            UtilityFunctions::printerr(vformat("玩家%d播放crouch_down动画时发生异常, 跳过", player_id));
        }
        return;
    }
    
    Ref<SpriteFrames> sf = animated_sprite->get_sprite_frames();
    if (!sf.is_valid()) {
        return;
    }
    
    // 检查动画是否存在
    if (!sf->has_animation(anim_name)) {
        UtilityFunctions::print(vformat("玩家%d: 动画不存在: %s", player_id, anim_name));
        return;
    }
    
    // 获取当前动画和目标动画
    StringName current_anim = animated_sprite->get_animation();
    StringName target_anim = StringName(anim_name);
    
    // 如果动画不同，或者强制重启，则播放新动画
    if (current_anim != target_anim || force_restart) {
        UtilityFunctions::print(vformat("玩家%d播放动画: %s", player_id, anim_name));
        animated_sprite->play(anim_name);
    }
}

bool Player::has_animation(const String& anim_name) {
    if (!animated_sprite) return false;
    
    Ref<SpriteFrames> sf = animated_sprite->get_sprite_frames();
    if (!sf.is_valid()) return false;
    
    return sf->has_animation(anim_name);
}

void Player::start_jump(JumpType jump_type) {
    Vector2 velocity = get_velocity();
    
    UtilityFunctions::print(vformat("玩家%d开始跳跃，类型: %s", player_id, jump_type == FORWARD_JUMP ? "前跳" : "上跳"));
    
    if (jump_type == FORWARD_JUMP) {
        velocity.y = -forward_jump_force;
        if (input_direction != 0) {
            velocity.x = input_direction * forward_jump_force * 0.5f;
        }
        play_animation("forward_jump", true);
    } else if (jump_type == UP_JUMP) {
        velocity.y = -up_jump_force;
        play_animation("up_jump", true);
    }
    
    set_velocity(velocity);
    is_jumping = true;
    current_jump_type = jump_type;
    
    // 跳跃时关闭奔跑状态
    is_running = false;
}

void Player::end_jump() {
    is_jumping = false;
    current_jump_type = NO_JUMP;
}

void Player::start_landing() {
    is_jumping = false;
    
    // 根据跳跃类型决定是否播放着陆动画
    if (current_jump_type == FORWARD_JUMP) {
        // forward_jump落地，不播放着陆动画
        is_landing = false;
        current_jump_type = NO_JUMP;
        
        // 如果玩家在落地时还在奔跑，保持奔跑状态
        if (run_pressed) {
            is_running = true;
        }
        
        UtilityFunctions::print(vformat("玩家%dforward_jump着陆, 不播放着陆动画", player_id));
    } else {
        // 其他跳跃类型落地，播放着陆动画
        is_landing = true;
        landing_timer = 0.2f;
        play_animation("landing", true);
        current_jump_type = NO_JUMP;
        UtilityFunctions::print(vformat("玩家%d开始着陆，播放着陆动画", player_id));
    }
}

void Player::take_damage(float damage) {
    if (is_dead || is_invincible) return;  // 已死亡或无敌状态下不再受到伤害
    
    current_health -= damage;
    
    UtilityFunctions::print(vformat("玩家%d受到伤害: %.1f, 剩余生命: %.1f/%.1f", 
        player_id, damage, current_health, max_health));
    
    // 受到伤害时，启动短暂无敌帧
    enable_invincibility();
    
    // 检查是否死亡
    if (current_health <= 0) {
        current_health = 0;
        die();  // 调用现有的die()方法，它会播放falling动画
    }
}

void Player::heal(float amount) {
    if (is_dead) return;  // 已死亡无法治疗
    
    current_health += amount;
    if (current_health > max_health) {
        current_health = max_health;
    }
    
    UtilityFunctions::print(vformat("玩家%d恢复生命: %.1f, 当前生命: %.1f/%.1f", 
        player_id, amount, current_health, max_health));
}

void Player::reset_health() {
    current_health = max_health;
    is_dead = false;  // 重置死亡状态
    
    // 确保无敌状态被重置
    disable_invincibility();
    
    UtilityFunctions::print(vformat("玩家%d生命值已重置", player_id));
}

void Player::die() {
    is_dead = true;
    set_velocity(Vector2(0, 0));
    play_animation("falling", true);
    
    // 死亡时禁用HurtBox
    if (hurtbox_area) {
        hurtbox_area->set_collision_layer(0);
        hurtbox_area->set_collision_mask(0);
    }
    
    UtilityFunctions::print(vformat("玩家%d死亡", player_id));
}

// 玩家标识
int Player::get_player_id() const { 
    return player_id; 
}

void Player::set_player_id(int id) { 
    player_id = id; 
}

// 属性getter/setter实现
float Player::get_walk_speed() const { return walk_speed; }
void Player::set_walk_speed(float speed) { walk_speed = speed; }
float Player::get_run_speed() const { return run_speed; }
void Player::set_run_speed(float speed) { run_speed = speed; }
float Player::get_jump_force() const { return jump_force; }
void Player::set_jump_force(float force) { jump_force = force; }
float Player::get_forward_jump_force() const { return forward_jump_force; }
void Player::set_forward_jump_force(float force) { forward_jump_force = force; }
float Player::get_up_jump_force() const { return up_jump_force; }
void Player::set_up_jump_force(float force) { up_jump_force = force; }
float Player::get_gravity() const { return gravity; }
void Player::set_gravity(float value) { gravity = value; }

String Player::get_animated_sprite_path() const { 
    return animated_sprite_path; 
}
void Player::set_animated_sprite_path(const String& path) { 
    animated_sprite_path = path; 
}

// 攻击属性getter/setter
float Player::get_attack_damage() const { return attack_damage; }
void Player::set_attack_damage(float damage) { attack_damage = damage; }
float Player::get_attack_range() const { return attack_range; }
void Player::set_attack_range(float range) { 
    attack_range = range; 
    update_attack_area_position();
}
float Player::get_attack_width() const { return attack_width; }
void Player::set_attack_width(float width) { 
    attack_width = width; 
    update_attack_area_position();
}
Vector2 Player::get_attack_knockback() const { return attack_knockback; }
void Player::set_attack_knockback(const Vector2& knockback) { 
    attack_knockback = knockback; 
}

// 无敌帧属性getter/setter
float Player::get_invincible_duration() const { return invincible_duration; }
void Player::set_invincible_duration(float duration) { 
    invincible_duration = duration; 
}
bool Player::get_is_invincible() const { 
    return is_invincible; 
}

float Player::get_max_health() const { 
    return max_health; 
}

void Player::set_max_health(float health) { 
    max_health = health; 
    if (current_health > max_health) {
        current_health = max_health;
    }
}

float Player::get_current_health() const { 
    return current_health; 
}

void Player::set_current_health(float health) { 
    current_health = health; 
    if (current_health <= 0) {
        current_health = 0;
        if (!is_dead) {
            die();
        }
    } else if (current_health > max_health) {
        current_health = max_health;
    }
}

float Player::get_health_percentage() const { 
    return (max_health > 0) ? (current_health / max_health) : 0.0f; 
}

bool Player::get_is_crouching() const { return is_crouching; }
bool Player::get_is_running() const { return is_running; }
bool Player::get_is_dead() const { return is_dead; }
bool Player::get_is_jumping() const { return is_jumping; }
bool Player::get_is_landing() const { return is_landing; }
int Player::get_current_jump_type() const { 
    return (int)current_jump_type;  
}
int Player::get_current_attack_type() const { 
    return (int)current_attack_type;  
}
bool Player::get_is_attacking() const { 
    return is_attacking;  
}

// 区域节点访问
Area2D* Player::get_attack_area() const { 
    return attack_area; 
}

Area2D* Player::get_hurtbox_area() const { 
    return hurtbox_area; 
}

} // namespace godot
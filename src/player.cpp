#include "player.h"
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/input.hpp>
#include <godot_cpp/classes/animated_sprite2d.hpp>
#include <godot_cpp/classes/sprite_frames.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {

Player::Player() : 
    walk_speed(200.0f),
    run_speed(400.0f),
    ladder_speed(150.0f),
    jump_force(400.0f),
    forward_jump_force(350.0f),
    up_jump_force(450.0f),
    roll_speed(500.0f),
    roll_duration(0.5f),
    roll_timer(0.0f),
    gravity(980.0f),
    is_crouching(false),       // 下蹲状态
    crouch_timer(0.0f),        // 下蹲计时器
    crouch_duration(0.3f),     // 下蹲动画持续时间（0.3秒）
    is_on_ladder(false),
    is_climbing(false),
    is_rolling(false),
    is_obstacle_climbing(false),
    is_dead(false),
    is_running(false),
    is_sprite_flipped_h(false),
    current_jump_type(NO_JUMP),
    is_jumping(false),
    is_landing(false),
    landing_timer(0.0f),
    input_direction(0.0f),
    vertical_input(0.0f),
    jump_pressed(false),
    run_pressed(false),
    crouch_pressed(false),
    crouch_was_pressed(false),
    roll_pressed(false),
    animated_sprite(nullptr)
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
    
    // 状态查询
    ClassDB::bind_method(D_METHOD("get_is_crouching"), &Player::get_is_crouching);
    ClassDB::bind_method(D_METHOD("get_is_running"), &Player::get_is_running);
    ClassDB::bind_method(D_METHOD("get_is_dead"), &Player::get_is_dead);
    ClassDB::bind_method(D_METHOD("get_is_jumping"), &Player::get_is_jumping);
    ClassDB::bind_method(D_METHOD("get_is_landing"), &Player::get_is_landing);
    ClassDB::bind_method(D_METHOD("get_current_jump_type"), &Player::get_current_jump_type);
    
    // 编辑器属性
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "walk_speed"), "set_walk_speed", "get_walk_speed");
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "run_speed"), "set_run_speed", "get_run_speed");
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "jump_force"), "set_jump_force", "get_jump_force");
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "forward_jump_force"), "set_forward_jump_force", "get_forward_jump_force");
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "up_jump_force"), "set_up_jump_force", "get_up_jump_force");
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "gravity"), "set_gravity", "get_gravity");
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
    
    // 8. 更新状态
    update_state();
    
    // 9. 更新动画
    update_animation();
    
    // 10. 更新翻滚计时
    if (is_rolling) {
        roll_timer -= delta;
        if (roll_timer <= 0) {
            is_rolling = false;
        }
    }
    
    // 11. 记录上一帧的crouch_pressed状态
    crouch_was_pressed = crouch_pressed;
    
    // 调试输出
    static int frame_count = 0;
    frame_count++;
    if (frame_count % 30 == 0) {
        UtilityFunctions::print(vformat(
            "状态: 地面=%s, 下蹲=%s, 计时器=%.2f, crouch_pressed=%s", 
            is_on_floor() ? "是" : "否",
            is_crouching ? "是" : "否",
            crouch_timer,
            crouch_pressed ? "是" : "否"
        ));
    }
}

void Player::handle_input() {
    Input *input = Input::get_singleton();
    
    // 水平移动
    input_direction = 0.0f;
    if (input->is_action_pressed("move_left")) input_direction -= 1.0f;
    if (input->is_action_pressed("move_right")) input_direction += 1.0f;
    
    // 垂直移动
    vertical_input = 0.0f;
    if (input->is_action_pressed("move_up")) vertical_input += 1.0f;
    if (input->is_action_pressed("move_down")) vertical_input -= 1.0f;
    
    // 动作按键
    jump_pressed = input->is_action_just_pressed("jump");
    run_pressed = input->is_action_pressed("run");
    crouch_pressed = input->is_action_pressed("crouch");
    roll_pressed = input->is_action_just_pressed("roll");
}

void Player::apply_movement(double delta) {
    if (is_dead) {
        set_velocity(Vector2(0, 0));
        return;
    }
    
    Vector2 velocity = get_velocity();
    
    // 应用重力
    if (!is_climbing && !is_dead) {
        velocity.y += gravity * delta;
    }
    
    if (is_rolling) {
        // 翻滚移动
        velocity.x = input_direction * roll_speed;
    } else if (is_climbing) {
        // 梯子移动
        velocity.x = 0;
        velocity.y = -vertical_input * ladder_speed;
    } else {
        // 根据是否在地面，选择不同的速度策略
        if (is_on_floor()) {
            // 地面移动
            // 下蹲状态下不允许移动
            if (!is_crouching) {
                float target_speed = walk_speed;
                if (is_running) target_speed = run_speed;
                
                velocity.x = input_direction * target_speed;
            } else {
                // 下蹲状态下，水平速度设为0
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
        if (!is_crouching) {
            // 开始下蹲
            is_crouching = true;
            crouch_timer = crouch_duration;
            UtilityFunctions::print("开始下蹲");
        }
    }
    
    // 检测按键松开
    if (!crouch_pressed && crouch_was_pressed) {
        if (is_crouching && crouch_timer <= 0) {
            // 开始起身，设置计时器但不改变is_crouching状态
            crouch_timer = crouch_duration;
            UtilityFunctions::print("开始起身，设置计时器");
        }
    }
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
    
    // 翻滚状态
    if (is_rolling) {
        play_animation("roll");
        return;
    }
    
    // 梯子状态
    if (is_climbing) {
        if (vertical_input > 0) {
            play_animation("up_ladder");
        } else if (vertical_input < 0) {
            play_animation("down_ladder");
        } else {
            play_animation("idle");
        }
        return;
    }
    
    // 攀爬障碍
    if (is_obstacle_climbing) {
        play_animation("obstacle");
        return;
    }
    
    // 处理跳跃触发
    if (jump_pressed && is_on_floor() && !is_jumping && !is_landing && !is_crouching) {
        UtilityFunctions::print("检测到跳跃按键，当前奔跑状态: " + String(is_running ? "是" : "否"));
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
                UtilityFunctions::print("起身动画播放完毕，结束下蹲状态");
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
    if (roll_pressed && is_on_floor() && !is_rolling && !is_crouching) {
        is_rolling = true;
        roll_timer = roll_duration;
    }
    
    // 奔跑状态更新
    if (run_pressed && !is_running && !is_crouching) {
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
                UtilityFunctions::print("播放crouch_down动画");
            }
        } catch (...) {
            UtilityFunctions::printerr("播放crouch_down动画时发生异常, 跳过");
        }
        return;
    }
    
    Ref<SpriteFrames> sf = animated_sprite->get_sprite_frames();
    if (!sf.is_valid()) {
        return;
    }
    
    // 检查动画是否存在
    if (!sf->has_animation(anim_name)) {
        UtilityFunctions::print(vformat("动画不存在: %s", anim_name));
        return;
    }
    
    // 获取当前动画和目标动画
    StringName current_anim = animated_sprite->get_animation();
    StringName target_anim = StringName(anim_name);
    
    // 如果动画不同，或者强制重启，则播放新动画
    if (current_anim != target_anim || force_restart) {
        UtilityFunctions::print(vformat("播放动画: %s", anim_name));
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
    
    UtilityFunctions::print(vformat("开始跳跃，类型: %s", jump_type == FORWARD_JUMP ? "前跳" : "上跳"));
    
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
        
        UtilityFunctions::print("forward_jump着陆, 不播放着陆动画");
    } else {
        // 其他跳跃类型落地，播放着陆动画
        is_landing = true;
        landing_timer = 0.2f;
        play_animation("landing", true);
        current_jump_type = NO_JUMP;
        UtilityFunctions::print("开始着陆，播放着陆动画");
    }
}

void Player::start_climbing_ladder(bool up) {
    is_climbing = true;
    is_on_ladder = true;
    set_velocity(Vector2(0, 0));
}

void Player::stop_climbing_ladder() {
    is_climbing = false;
    is_on_ladder = false;
}

void Player::climb_obstacle() {
    if (!is_obstacle_climbing && is_on_floor()) {
        is_obstacle_climbing = true;
    }
}

void Player::die() {
    is_dead = true;
    set_velocity(Vector2(0, 0));
    play_animation("falling", true);
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

bool Player::get_is_crouching() const { return is_crouching; }
bool Player::get_is_running() const { return is_running; }
bool Player::get_is_dead() const { return is_dead; }
bool Player::get_is_jumping() const { return is_jumping; }
bool Player::get_is_landing() const { return is_landing; }
int Player::get_current_jump_type() const { 
    return (int)current_jump_type;  
}

} // namespace godot
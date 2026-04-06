#ifndef PLAYER_H
#define PLAYER_H

#include <godot_cpp/classes/character_body2d.hpp>
#include <godot_cpp/classes/animated_sprite2d.hpp>
#include <godot_cpp/core/class_db.hpp>

namespace godot {

class Player : public CharacterBody2D {
    GDCLASS(Player, CharacterBody2D)

public:
    // 跳跃类型枚举
    enum JumpType {
        NO_JUMP = 0,
        UP_JUMP = 1,
        FORWARD_JUMP = 2
    };

private:
    // 移动属性
    float walk_speed;
    float run_speed;
    float ladder_speed;
    float jump_force;
    float forward_jump_force;
    float up_jump_force;
    float roll_speed;
    float roll_duration;
    float roll_timer;
    float gravity;
    
    // 状态标志
    bool is_crouching;        // 下蹲状态
    float crouch_timer;       // 下蹲计时器
    float crouch_duration;    // 下蹲动画持续时间
    bool is_on_ladder;
    bool is_climbing;
    bool is_rolling;
    bool is_obstacle_climbing;
    bool is_dead;
    bool is_running;
    bool is_sprite_flipped_h;
    
    // 跳跃状态
    JumpType current_jump_type;
    bool is_jumping;
    bool is_landing;
    float landing_timer;
    
    // 输入状态
    float input_direction;
    float vertical_input;
    bool jump_pressed;
    bool run_pressed;
    bool crouch_pressed;
    bool crouch_was_pressed;
    bool roll_pressed;
    
    // 动画控制
    AnimatedSprite2D* animated_sprite;
    String animated_sprite_path;

protected:
    static void _bind_methods();

public:
    Player();
    ~Player();

    // 生命周期
    void _ready() override;
    void _physics_process(double delta) override;
    
    // 核心方法
    void handle_input();
    void update_state();
    void apply_movement(double delta);
    void update_animation();
    
    // 动画控制
    void play_animation(const String& anim_name, bool force_restart = false);
    bool has_animation(const String& anim_name);
    
    // 下蹲控制
    void handle_crouch(double delta);
    
    // 跳跃控制
    void start_jump(JumpType jump_type);
    void end_jump();
    void start_landing();
    
    // 特殊动作
    void start_climbing_ladder(bool up);
    void stop_climbing_ladder();
    void climb_obstacle();
    void die();
    
    // 属性访问
    float get_walk_speed() const;
    void set_walk_speed(float speed);
    float get_run_speed() const;
    void set_run_speed(float speed);
    float get_jump_force() const;
    void set_jump_force(float force);
    float get_forward_jump_force() const;
    void set_forward_jump_force(float force);
    float get_up_jump_force() const;
    void set_up_jump_force(float force);
    float get_gravity() const;
    void set_gravity(float value);
    
    String get_animated_sprite_path() const;
    void set_animated_sprite_path(const String& path);
    
    bool get_is_crouching() const;
    bool get_is_running() const;
    bool get_is_dead() const;
    bool get_is_jumping() const;
    bool get_is_landing() const;
    int get_current_jump_type() const;
};

} // namespace godot

#endif // PLAYER_H
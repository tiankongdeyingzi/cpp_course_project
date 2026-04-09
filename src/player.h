#ifndef PLAYER_H
#define PLAYER_H

#include <godot_cpp/classes/character_body2d.hpp>
#include <godot_cpp/classes/animated_sprite2d.hpp>
#include <godot_cpp/classes/area2d.hpp>
#include <godot_cpp/classes/collision_shape2d.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/array.hpp>

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
    
    // 攻击类型枚举
    enum AttackType {
        NO_ATTACK = 0,
        PUNCH_1 = 1,    // 左拳
        PUNCH_2 = 2     // 右拳
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
    bool is_dead;
    bool is_running;
    bool is_sprite_flipped_h;
    
    // 攻击状态
    AttackType current_attack_type;  // 当前攻击类型
    float attack_timer;              // 攻击计时器
    float attack_duration;           // 攻击动画持续时间
    bool is_attacking;               // 是否正在攻击
    float attack_damage;             // 攻击伤害值
    Vector2 attack_knockback;        // 攻击击退力

    //血量管理
    float max_health;         // 最大生命值
    float current_health;     // 当前生命值
    float health_regen_rate;  // 生命恢复速率
    float health_regen_timer; // 生命恢复计时器
    
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
    bool left_mouse_pressed;    // 鼠标左键
    bool right_mouse_pressed;   // 鼠标右键
    
    // 动画控制
    AnimatedSprite2D* animated_sprite;
    String animated_sprite_path;
    
    // 攻击检测区域 - 引用您手动创建的AttackArea节点
    Area2D* attack_area;
    // 受击区域 - 引用您手动创建的HurtBox节点
    Area2D* hurtbox_area;
    
    // 攻击范围参数
    float attack_range;          // 攻击范围
    float attack_width;          // 攻击宽度
    
    // 命中检测相关
    Array hit_targets;           // 已命中的目标（防止重复命中）
    
    // 攻击检测方法
    void update_attack_area_position();
    void enable_attack_detection();
    void disable_attack_detection();
    void process_hit_detection();
    void reset_hit_targets();

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
    
    // 攻击控制
    void handle_attack(double delta);
    void start_attack(AttackType attack_type);
    void end_attack();
    void apply_attack_knockback(Node* target);
    
    // 血量管理
    void take_damage(float damage);
    void heal(float amount);
    void reset_health();
    float get_max_health() const;
    void set_max_health(float health);
    float get_current_health() const;
    void set_current_health(float health);
    float get_health_percentage() const;
    
    // 跳跃控制
    void start_jump(JumpType jump_type);
    void end_jump();
    void start_landing();
    
    // 特殊动作
    void start_climbing_ladder(bool up);
    void stop_climbing_ladder();
    
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
    
    // 攻击属性
    float get_attack_damage() const;
    void set_attack_damage(float damage);
    float get_attack_range() const;
    void set_attack_range(float range);
    float get_attack_width() const;
    void set_attack_width(float width);
    Vector2 get_attack_knockback() const;
    void set_attack_knockback(const Vector2& knockback);
    
    // 状态查询
    bool get_is_crouching() const;
    bool get_is_running() const;
    bool get_is_dead() const;
    bool get_is_jumping() const;
    bool get_is_landing() const;
    int get_current_jump_type() const;
    int get_current_attack_type() const;
    bool get_is_attacking() const;
    
    // 区域节点访问（用于调试）
    Area2D* get_attack_area() const;
    Area2D* get_hurtbox_area() const;
};

} // namespace godot

#endif // PLAYER_H
#ifndef BSOR_H_
#define BSOR_H_

#include <stdint.h>
#include <stdbool.h>

typedef enum _BSOR_Tracking {
    BSOR_TRACK_OPENVR,
    BSOR_TRACK_OCULUS,
    BSOR_TRACK_UNKNOWN = -1
} BSOR_Tracking;

typedef enum _BSOR_Platform {
    BSOR_PLAT_STEAM,
    BSOR_PLAT_OCULUS,
    BSOR_PLAT_UNKNOWN = -1
} BSOR_Platform;

typedef enum _BSOR_Difficulty {
    BSOR_DIFF_EASY,
    BSOR_DIFF_MEDIUM,
    BSOR_DIFF_HARD,
    BSOR_DIFF_EXPERT,
    BSOR_DIFF_EXPERTPLUS,
    BSOR_DIFF_UNKNOWN = -1
} BSOR_Difficulty;

typedef enum _BSOR_NoteType {
    BSOR_NOTE_GOOD,
    BSOR_NOTE_BAD,
    BSOR_NOTE_MISS,
    BSOR_NOTE_BOMB,
    BSOR_NOTE_UNKNOWN = -1
} BSOR_NoteType;

typedef struct _BSOR_MapMetadata {
    char * map_hash;
    char * song_name;
    char * charter;
    BSOR_Difficulty difficulty;
} BSOR_MapMetadata;

typedef struct _BSOR_SystemMetadata {
    uint8_t file_version;
    char * mod_version;
    char * game_version;
} BSOR_SystemMetadata;

typedef struct _BSOR_PlatformMetadata {
    BSOR_Platform type;
    char * user_id;
    char * user_name;
} BSOR_PlatformMetadata;

typedef struct _BSOR_VRMetadata {
    BSOR_Tracking type;
    char * hmd;
    char * controller_r;
} BSOR_VRMetadata;

typedef struct _BSOR_PlayMetadata {
    uint64_t start_timestamp;
    uint32_t score;
    char *game_mode;
    char *environment;
    char *modifiers;
    float song_speed;
    float practice_start;
    float fail_time;
    float player_height;
    float jump_distance;
    bool left_handed;
} BSOR_PlayMetadata;


typedef struct _BSOR_XYZ {
    float x;
    float y;
    float z;
} BSOR_XYZ;

typedef struct _BSOR_XYZW {
    float x;
    float y;
    float z;
    float w;
} BSOR_XYZW;

typedef struct _BSOR_Frame {
    float song_time;
    int fps;
    BSOR_XYZ head_pos;
    BSOR_XYZW head_rotation;
    BSOR_XYZ hand_l_pos;
    BSOR_XYZW hand_l_rotation;
    BSOR_XYZ hand_r_pos;
    BSOR_XYZW hand_r_rotation;
} BSOR_Frame;

typedef struct _BSOR_Note {
    int note_id;
    float event_time;
    float spawn_time;
    BSOR_NoteType event;
    bool speed_ok;
    bool direction_ok;
    bool saber_ok;
    bool cut_too_soon;
    float saber_speed;
    BSOR_XYZ saber_dir;
    int saber_type;
    float time_deviation;
    float dir_deviation;
    BSOR_XYZ cut_point;
    BSOR_XYZ cut_normal;
    float cut_distance_to_center;
    float cut_angle;
    float cut_before_rating;
    float cut_after_rating;
} BSOR_Note;

typedef struct _BSOR_Wall {
    int wall_id;
    float energy;
    float event_time;
    float spawn_time;
} BSOR_Wall;

typedef struct _BSOR_Height {
    float height;
    float song_time;
} BSOR_Height;

typedef struct _BSOR_Pause {
    int duration;
    float song_time;
} BSOR_Pause;

typedef struct _BSOR {
    // static values
    BSOR_SystemMetadata system;
    BSOR_PlatformMetadata platform;
    BSOR_VRMetadata vr;
    BSOR_MapMetadata map;
    BSOR_PlayMetadata play;
    // dynamic sized values
    int frame_count;
    BSOR_Frame * frames;
    int note_count;
    BSOR_Note * notes;
    int wall_count;
    BSOR_Wall * walls;
    int height_count;
    BSOR_Height * heights;
    int pause_count;
    BSOR_Pause *pauses;
} BSOR;

#endif // BSOR_H_
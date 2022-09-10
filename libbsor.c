#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include "bsor_internal.h"

bool bsor_read_bool_file(FILE *fp) {
    char read_byte = fgetc(fp);
    return (read_byte > 0);
}
uint8_t bsor_read_char_file(FILE *fp) {
    return (uint8_t)fgetc(fp);
}
int bsor_read_int_file(FILE *fp) {
    int read_int = 0;
    fread(&read_int, sizeof(int), 1, fp);
    return read_int;
}
float bsor_read_float_file(FILE *fp) {
    float read_float = 0.00f;
    fread(&read_float, sizeof(float), 1, fp);
    return read_float;
}
char * bsor_read_string_file(FILE *fp) {
    int string_length = 0;
    char * string = NULL;
    // strings are stored as [length][string] so read the length first
    fread(&string_length, sizeof(int), 1, fp);
    // hacky workaround for broken unicode
    if (string_length < 0 || string_length > 0x200) {
        fseek(fp, -3, SEEK_CUR);
        return bsor_read_string_file(fp);
    }
    // allocate some memory for the string and read the string afterwards
    string = malloc(string_length + 1);
    if (string != NULL) {
        fread(string, 1, string_length, fp);
        string[string_length] = 0; // cap off the string with a null byte
    }
    // return the string we allocated
    return string;
}
void bsor_read_xyz_file(FILE *fp, BSOR_XYZ *xyz) {
    xyz->x = bsor_read_float_file(fp);
    xyz->y = bsor_read_float_file(fp);
    xyz->z = bsor_read_float_file(fp);
}
void bsor_read_xyzw_file(FILE *fp, BSOR_XYZW *xyzw) {
    xyzw->x = bsor_read_float_file(fp);
    xyzw->y = bsor_read_float_file(fp);
    xyzw->z = bsor_read_float_file(fp);
    xyzw->w = bsor_read_float_file(fp);
}

void bsor_read_metadata_file(FILE *fp, BSOR *bsor) {
    // the start of the metadata section should be 0x00
    uint8_t metadata_magic = fgetc(fp);
    if (metadata_magic != 0)
        return;
    // read system metadata (game + mod versions)
    bsor->system.mod_version = bsor_read_string_file(fp);
    bsor->system.game_version = bsor_read_string_file(fp);
    // read play start time and convert to integer
    char *start_time_str = bsor_read_string_file(fp);
    sscanf(start_time_str, "%llu", &bsor->play.start_timestamp);
    free(start_time_str);
    // read platform metadata
    bsor->platform.type = BSOR_PLAT_UNKNOWN;
    bsor->platform.user_id = bsor_read_string_file(fp);
    bsor->platform.user_name = bsor_read_string_file(fp);
    // read platform type, convert to enum
    char *platform = bsor_read_string_file(fp);
    if (strcmp(platform, "steam") == 0)
        bsor->platform.type = BSOR_PLAT_STEAM;
    if (strcmp(platform, "oculus") == 0)
        bsor->platform.type = BSOR_PLAT_OCULUS;
    free(platform);
    // read VR system metadata
    // read type, convert to enum
    bsor->vr.type = BSOR_TRACK_UNKNOWN;
    char *tracking = bsor_read_string_file(fp);
    if (strcmp(tracking, "OpenVR") == 0)
        bsor->vr.type = BSOR_TRACK_OPENVR;
    if (strcmp(tracking, "Oculus") == 0)
        bsor->vr.type = BSOR_TRACK_OCULUS;
    free(tracking);
    // read hmd and controller type
    bsor->vr.hmd = bsor_read_string_file(fp);
    bsor->vr.controller_r = bsor_read_string_file(fp);
    // read song metadata
    bsor->map.map_hash = bsor_read_string_file(fp);
    bsor->map.song_name = bsor_read_string_file(fp);
    bsor->map.charter = bsor_read_string_file(fp);
    // read difficulty, convert to enum
    bsor->map.difficulty = BSOR_DIFF_UNKNOWN;
    char *difficulty = bsor_read_string_file(fp);
    if (strcmp(difficulty, "Easy") == 0)
        bsor->map.difficulty = BSOR_DIFF_EASY;
    if (strcmp(difficulty, "Medium") == 0)
        bsor->map.difficulty = BSOR_DIFF_MEDIUM;
    if (strcmp(difficulty, "Hard") == 0)
        bsor->map.difficulty = BSOR_DIFF_HARD;
    if (strcmp(difficulty, "Expert") == 0)
        bsor->map.difficulty = BSOR_DIFF_EXPERT;
    if (strcmp(difficulty, "ExpertPlus") == 0)
        bsor->map.difficulty = BSOR_DIFF_EXPERTPLUS;
    free(difficulty);
    // read play metadata
    bsor->play.score = bsor_read_int_file(fp);
    bsor->play.game_mode = bsor_read_string_file(fp);
    bsor->play.environment = bsor_read_string_file(fp);
    bsor->play.modifiers = bsor_read_string_file(fp);
    bsor->play.jump_distance = bsor_read_float_file(fp);
    bsor->play.left_handed = bsor_read_bool_file(fp);
    bsor->play.player_height = bsor_read_float_file(fp);
    bsor->play.practice_start = bsor_read_float_file(fp);
    bsor->play.fail_time = bsor_read_float_file(fp);
    bsor->play.song_speed = bsor_read_float_file(fp);
}

void bsor_read_frames_file(FILE *fp, BSOR *bsor) {
    // the start of the frames section should be 0x01
    uint8_t frames_magic = fgetc(fp);
    if (frames_magic != 1)
        return;
    // read the frame count
    fread(&bsor->frame_count, sizeof(int), 1, fp);
    // allocate the frame data
    bsor->frames = malloc(bsor->frame_count * sizeof(BSOR_Frame));
    // if we failed to allocate, bail out
    if (bsor->frames == NULL)
        return;
    // run through each frame data
    for (int i = 0; i < bsor->frame_count; i++) {
        // read song time and fps
        bsor->frames[i].song_time = bsor_read_float_file(fp);
        bsor->frames[i].fps = bsor_read_int_file(fp);
        // read head position + rotation
        bsor_read_xyz_file(fp, &bsor->frames[i].head_pos);
        bsor_read_xyzw_file(fp, &bsor->frames[i].head_rotation);
        // read left hand position + rotation
        bsor_read_xyz_file(fp, &bsor->frames[i].hand_l_pos);
        bsor_read_xyzw_file(fp, &bsor->frames[i].hand_l_rotation);
        // read right hand position + rotation
        bsor_read_xyz_file(fp, &bsor->frames[i].hand_r_pos);
        bsor_read_xyzw_file(fp, &bsor->frames[i].hand_r_rotation);
    }
}

void bsor_read_notes_file(FILE *fp, BSOR *bsor) {
    // the start of the notes section should be 0x02
    uint8_t notes_magic = fgetc(fp);
    if (notes_magic != 2)
        return;
    // read the note count
    fread(&bsor->note_count, sizeof(int), 1, fp);
    // allocate the note data
    bsor->notes = malloc(bsor->note_count * sizeof(BSOR_Note));
    // if we failed to allocate, bail out
    if (bsor->notes == NULL)
        return;
    // run through the frame data
    for (int i = 0; i < bsor->note_count; i++) {
        bsor->notes[i].note_id = bsor_read_int_file(fp);
        bsor->notes[i].event_time = bsor_read_float_file(fp);
        bsor->notes[i].spawn_time = bsor_read_float_file(fp);
        bsor->notes[i].event = bsor_read_int_file(fp);
        if (bsor->notes[i].event == BSOR_NOTE_GOOD || bsor->notes[i].event == BSOR_NOTE_BAD) {
            bsor->notes[i].speed_ok = bsor_read_bool_file(fp);
            bsor->notes[i].direction_ok = bsor_read_bool_file(fp);
            bsor->notes[i].saber_ok = bsor_read_bool_file(fp);
            bsor->notes[i].cut_too_soon = bsor_read_bool_file(fp);
            bsor->notes[i].saber_speed = bsor_read_float_file(fp);
            bsor_read_xyz_file(fp, &bsor->notes[i].saber_dir);
            bsor->notes[i].saber_type = bsor_read_int_file(fp);
            bsor->notes[i].time_deviation = bsor_read_float_file(fp);
            bsor->notes[i].dir_deviation = bsor_read_float_file(fp);
            bsor_read_xyz_file(fp, &bsor->notes[i].cut_point);
            bsor_read_xyz_file(fp, &bsor->notes[i].cut_normal);
            bsor->notes[i].cut_distance_to_center = bsor_read_float_file(fp);
            bsor->notes[i].cut_angle = bsor_read_float_file(fp);
            bsor->notes[i].cut_before_rating = bsor_read_float_file(fp);
            bsor->notes[i].cut_after_rating = bsor_read_float_file(fp);
        }
    }
}

void bsor_read_walls_file(FILE *fp, BSOR *bsor) {
    // the start of the walls section should be 0x03
    uint8_t walls_magic = fgetc(fp);
    if (walls_magic != 3)
        return;
    // read the note count
    fread(&bsor->wall_count, sizeof(int), 1, fp);
    // allocate the walls data
    bsor->walls = malloc(bsor->wall_count * sizeof(BSOR_Wall));
    // if we failed to allocate, bail out
    if (bsor->walls == NULL)
        return;
    // run through the walls data
    for (int i = 0; i < bsor->wall_count; i++) {
        bsor->walls[i].wall_id = bsor_read_int_file(fp);
        bsor->walls[i].energy = bsor_read_float_file(fp);
        bsor->walls[i].event_time = bsor_read_float_file(fp);
        bsor->walls[i].spawn_time = bsor_read_float_file(fp);
    }
}

void bsor_read_heights_file(FILE *fp, BSOR *bsor) {
    // the start of the heights section should be 0x04
    uint8_t heights_magic = fgetc(fp);
    if (heights_magic != 4)
        return;
    // read the note count
    fread(&bsor->height_count, sizeof(int), 1, fp);
    // allocate the heights data
    bsor->heights = malloc(bsor->height_count * sizeof(BSOR_Height));
    // if we failed to allocate, bail out
    if (bsor->heights == NULL)
        return;
    // run through the heights data
    for (int i = 0; i < bsor->height_count; i++) {
        bsor->heights[i].height = bsor_read_float_file(fp);
        bsor->heights[i].song_time = bsor_read_float_file(fp);
    }
}

void bsor_read_pauses_file(FILE *fp, BSOR *bsor) {
    // the start of the pauses section should be 0x05
    uint8_t pauses_magic = fgetc(fp);
    if (pauses_magic != 5)
        return;
    // read the note count
    fread(&bsor->pause_count, sizeof(int), 1, fp);
    // allocate the heights data
    bsor->pauses = malloc(bsor->pause_count * sizeof(BSOR_Pause));
    // if we failed to allocate, bail out
    if (bsor->pauses == NULL)
        return;
    // run through the heights data
    for (int i = 0; i < bsor->pause_count; i++) {
        bsor->pauses[i].duration = bsor_read_int_file(fp);
        bsor->pauses[i].song_time = bsor_read_float_file(fp);
    }
}

BSOR * bsor_load_file(FILE *fp) {
    // check if we've been passed a valid file pointer
    if (fp == NULL)
        return NULL;
    // check the file magic on the passed file
    int magic = 0;
    fread(&magic, sizeof(int), 1, fp);
    if (magic != 0x442d3d69)
        return NULL;
    // allocate a buffer for our bsor object
    BSOR *bsor = malloc(sizeof(BSOR));
    // if we couldn't allocate the bsor, nope out
    if (bsor == NULL)
        return NULL;
    // read the file version, and bail out if it isn't v1
    bsor->system.file_version = fgetc(fp);
    if (bsor->system.file_version != 1) {
        free(bsor);
        return NULL;
    }
    // load the metadata from the file
    bsor_read_metadata_file(fp, bsor);
    // load the frames from the file
    bsor_read_frames_file(fp, bsor);
    // load the notes from the file
    bsor_read_notes_file(fp, bsor);
    // load the walls from the file
    bsor_read_walls_file(fp, bsor);
    // load the heights from the file
    bsor_read_heights_file(fp, bsor);
    // load the pauses from the file
    bsor_read_pauses_file(fp, bsor);
    return bsor;
}

void bsor_free(BSOR *bsor) {
    // TODO: are we really freeing *everything*?
    // free dynamic lists
    free(bsor->frames);
    free(bsor->notes);
    free(bsor->walls);
    free(bsor->heights);
    free(bsor->pauses);
    // free strings (system)
    free(bsor->system.game_version);
    free(bsor->system.mod_version);
    // free strings (platform)
    free(bsor->platform.user_id);
    free(bsor->platform.user_name);
    // free strings (vr)
    free(bsor->vr.hmd);
    free(bsor->vr.controller_r);
    // free strings (map)
    free(bsor->map.map_hash);
    free(bsor->map.charter);
    free(bsor->map.song_name);
    // free strings (play)
    free(bsor->play.game_mode);
    free(bsor->play.environment);
    free(bsor->play.modifiers);
    // free the bsor object itself
    free(bsor);
}

int bsor_get_fps_at_time(BSOR *bsor, float time) {
    int fps = 0;
    float that_time = 0;
    for (int i = 0; i < bsor->frame_count; i++) {
        // if it's an exact match we can just return now
        if (time == bsor->frames[i].song_time)
            return bsor->frames[i].fps;
        // otherwise find the nearest time
        if (bsor->frames[i].song_time > that_time && time < bsor->frames[i].song_time) {
            fps = bsor->frames[i].fps;
            that_time = bsor->frames[i].song_time;
        }
    }
    return fps;
}

int main(int argc, char **argv) {
    FILE *fp = fopen(argv[1], "rb");
    BSOR *bsor = bsor_load_file(fp);
    fclose(fp);
    printf("Version: Game = %s, Mod = %s\n", bsor->system.game_version, bsor->system.mod_version);
    // print song details
    printf("Song: %s (Charter: %s)\n", bsor->map.song_name, bsor->map.charter);
    printf("Map Hash: %s\n", bsor->map.map_hash);
    // print player information
    printf("Player: %s ", bsor->platform.user_name);
    if (bsor->platform.type == BSOR_PLAT_STEAM)
        printf("(Steam ID: %s)\n", bsor->platform.user_id);
    else if (bsor->platform.type == BSOR_PLAT_OCULUS)
        printf("(Oculus ID: %s)\n", bsor->platform.user_id);
    else
        printf("(ID: %s)\n", bsor->platform.user_id);
    // print headset information
    if (bsor->vr.type == BSOR_TRACK_OPENVR)
        printf("SteamVR ");
    else if (bsor->vr.type == BSOR_TRACK_OCULUS)
        printf("Oculus ");
    printf("HMD: %s\n", bsor->vr.hmd);
    printf("Controller: %s\n", bsor->vr.controller_r);
    // print timestamp
    char time_string[40];
    struct tm local_time;
    localtime_r((time_t *)&bsor->play.start_timestamp, &local_time);
    strftime(time_string, sizeof(time_string), "%c", &local_time);
    printf("Time: %s\n", time_string);
    // print misc data
    printf("Game Mode: %s\n", bsor->play.game_mode);
    printf("Modifiers: %s\n", bsor->play.modifiers);
    // get average + lowest fps
    uint64_t total_fps = 0;
    for (int i = 0; i < bsor->frame_count; i++)
        total_fps += bsor->frames[i].fps;
    printf("Average FPS: %.2f\n", (double)total_fps / (double)bsor->frame_count);
    // print notes information
    printf("Total Notes: %i\n", bsor->note_count);
    int good_notes = 0;
    int bad_notes = 0;
    int miss_notes = 0;
    int bomb_hits = 0;
    for (int i = 0; i < bsor->note_count; i++) {
        if (bsor->notes[i].event == BSOR_NOTE_GOOD)
            good_notes++;
        if (bsor->notes[i].event == BSOR_NOTE_BAD)
            bad_notes++;
        if (bsor->notes[i].event == BSOR_NOTE_MISS)
            miss_notes++;
        if (bsor->notes[i].event == BSOR_NOTE_BOMB)
            bomb_hits++;
    }
    printf("Notes Hit: %i good, %i bad\n", good_notes, bad_notes);
    printf("Notes Missed: %i\n", miss_notes);
    printf("Bombs Hit: %i\n", bomb_hits);
    // todo: print other useful information
    // free our bsor object
    bsor_free(bsor);
    return 0;
}
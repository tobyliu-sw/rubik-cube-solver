/*`
 *   Copyright 2017 Toby Liu
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 */
#include "rubik_cube.hpp"

#include <iostream>
#include <cstring>
#include <cassert>
#include <cstdlib>
#include <ctime>
#include <map>

using namespace rb;

static const char* face_chars = "ULFRBD";

static const int face_num = 6;

enum FACE_CORNER {
    UL = 4,
    UR,
    DR,
    DL,
};

struct SliceInfo {
    int face_idx;     // face index
    int start_pos;    // start position
    int dir;          // index increase or decrease
    bool is_row;      // indices are along the row or col
};


static const SliceInfo slice_info[3][4] = {
    {{0, UL,  1, false}, {2, UL,  1, false}, {5, UL,  1, false}, {4, DR, -1, false}},   // L, l, X, r, R
    {{1, UL,  1,  true}, {2, UL,  1,  true}, {3, UL,  1,  true}, {4, UL,  1,  true}},   // U, u, Y, d, D
    {{0, UR, -1,  true}, {1, UL,  1, false}, {5, DL,  1,  true}, {3, DR, -1, false}},   // F, f, Z, b, B
};


inline int GetSliceInfoIndex(int move_char_idx) {
    switch (move_char_idx) {
      case L: case l: case X: case r: case R:
        return 0;
      case U: case u: case Y: case d: case D:
        return 1;
      case F: case f: case Z: case b: case B:
        return 2;
      default:
        assert(0);
    }
}


inline int StrChrIdx(const char* str, const char& ch, const int& str_len) {
    for (int i = 0; i < str_len; i ++)
        if (str[i] == ch)
            return i;
    return -1;
}


CUBE_FACE rb::CvtFaceCharToFace(const char& face_char) {
    int index = StrChrIdx(face_chars, face_char, strlen(face_chars));
    if (index < 0)
        return UNKNOWN_FACE;
    return (CUBE_FACE)index;
}


RubikCube::RubikCube(int dim/* = 3*/):
    dim_(dim), piece_num_(dim * dim) {

    face_mappings_ = new char[face_num + 1];
    std::strcpy(face_mappings_, face_chars);

    color_mappings_ = new char[face_num + 1];
    std::strcpy(color_mappings_, "WOGRBY");

    faces_ = new char[piece_num_ * face_num + 1];
    for (int i = 0; i < face_num; i ++) {
        char *face = &(faces_[piece_num_ * i]);
        std::memset(face, face_chars[i], piece_num_);
    }
    faces_[piece_num_ * face_num] = '\0';
}


RubikCube::RubikCube(const char* colors, int dim/* = 3*/):
    dim_(dim), piece_num_(dim * dim) {

    face_mappings_ = new char[face_num + 1];
    std::strcpy(face_mappings_, face_chars);

    color_mappings_ = new char[face_num + 1];
    MapColors(colors);

    faces_ = new char[piece_num_ * face_num + 1];
    for (int i = 0; i < face_num; i ++) {
        char *face = &(faces_[piece_num_ * i]);
        const char *face_colors = &(colors[piece_num_ * i]);
        for (int j = 0; j < piece_num_; j ++) {
            face[j] = ColorToFaceChar(face_colors[j]);
        }
    }
    faces_[piece_num_ * face_num] = '\0';
}


RubikCube::~RubikCube() {
    delete [] faces_;
    delete [] color_mappings_;
    delete [] face_mappings_;
}


RubikCube::RubikCube(const RubikCube& other):
    dim_(other.dim_), piece_num_(other.piece_num_) {

    face_mappings_ = new char[face_num + 1];
    std::strcpy(face_mappings_, other.face_mappings_);

    color_mappings_ = new char[face_num + 1];
    std::strcpy(color_mappings_, other.color_mappings_);

    faces_ = new char[piece_num_ * face_num + 1];
    std::strcpy(faces_, other.faces_);
}


RubikCube& RubikCube::operator=(const RubikCube& other) {
    delete [] faces_;
    delete [] color_mappings_;
    delete [] face_mappings_;

    dim_ = other.dim_;
    piece_num_ = other.piece_num_;

    face_mappings_ = new char[face_num + 1];
    std::strcpy(face_mappings_, other.face_mappings_);

    color_mappings_ = new char[face_num + 1];
    std::strcpy(color_mappings_, other.color_mappings_);

    faces_ = new char[piece_num_ * face_num + 1];
    std::strcpy(faces_, other.faces_);

    return *this;
}


void RubikCube::MapColors(const char* colors) {
    if (dim_ == 3 || dim_ == 5) {
        for (int i = 0; i < face_num; i ++) {
            color_mappings_[i] = colors[(i * piece_num_) + (piece_num_ >> 1)];
        }
    } else { // dim_ == 4
        static const int corner_coords[8][6] = {
            {0, 12, 1,  3, 2,  0}, // ULF
            {0, 15, 2,  3, 3,  0}, // UFR
            {0,  3, 3,  3, 4,  0}, // URB
            {0,  0, 4,  3, 1,  0}, // UBL
            {5,  0, 1, 15, 2, 12}, // DLF
            {5,  3, 2, 15, 3, 12}, // DFR
            {5, 15, 3, 15, 4, 12}, // DRB
            {5, 12, 4, 15, 1, 12}, // DBL
        };

        std::map<char,int> color_index_map;
        int color_map_idx = 0;
        for (int i = 0; i < 6; i += 2) {
            char color_char = colors[corner_coords[0][i] * piece_num_ + corner_coords[0][i + 1]];
            color_index_map[color_char] = color_map_idx;
            color_mappings_[color_map_idx++] = color_char;
        }

        while (color_map_idx < 6) {
            const int target_mask = (1 << (color_map_idx - 1)) | ((color_map_idx < 5)? 1: 2);
            for (int c = 0; c < 8; c ++) {
                int corner_mask = 0;
                char new_color_char = '\0';

                for (int i = 0; i < 6; i += 2) {
                    char color_char = colors[corner_coords[c][i] * piece_num_ + corner_coords[c][i + 1]];
                    if (color_index_map.find(color_char) != color_index_map.end()) {
                        corner_mask |= 1 << (color_index_map[color_char]);
                    } else {
                        new_color_char = color_char;
                    }
                }

                if (corner_mask == target_mask) {
                    color_index_map[new_color_char] = color_map_idx;
                    color_mappings_[color_map_idx++] = new_color_char;
                    break;
                }
            }
        }
    }
    color_mappings_[face_num] = '\0';
}

void RubikCube::Dump(const bool& is_color/* = false*/) {
    char line[4 * (dim_ + 1) + 1];

    // Dump UP face
    for (int i = 0; i < dim_; i ++) {
        std::memset(line, ' ', dim_ + 1);
        char *p = &(line[dim_ + 1]);
        for (int j = 0; j < dim_; j ++)
            *p++ = GetPieceChar(U, i, j, is_color);
        *p = '\0';
        std::cout << line << std::endl;
    }

    // Dump LEFT, FRONT, RIGHT, BACK faces
    for (int i = 0; i < dim_; i ++) {
        char *p = line;
        for (int f = (int)L; f <= (int)B; f ++) {
            for (int j = 0; j < dim_; j ++)
                *p++ = GetPieceChar((CUBE_FACE)f, i, j, is_color);
            *p++ = ' ';
        }
        *p = '\0';
        std::cout << line << std::endl;
    }

    // Dump DOWN face
    for (int i = 0; i < dim_; i ++) {
        std::memset(line, ' ', dim_ + 1);
        char *p = &(line[dim_ + 1]);
        for (int j = 0; j < dim_; j ++)
            *p++ = GetPieceChar(D, i, j, is_color);
        *p = '\0';
        std::cout << line << std::endl;
    }
    std::cout << std::endl;
}


char RubikCube::GetPieceChar(const CUBE_FACE& cube_face, const int& row, const int& col, const bool& is_color) {
    char ret_char = faces_[((int)cube_face * dim_ + row) * dim_ + col];
    if (is_color) {
        ret_char = FaceCharToColor(ret_char);
    }
    return ret_char;
}


char RubikCube::ColorToFaceChar(const char& color) {
    int index = StrChrIdx(color_mappings_, color, std::strlen(color_mappings_));
    assert(index >= 0);
    return face_chars[index];
}


char RubikCube::FaceCharToColor(const char& face_char) {
    return color_mappings_[CvtFaceCharToFace(face_char)];
}


void RubikCube::RotateCube(const ROTATE_CUBE_DIR& dir) {
    static const CUBE_FACE rotate_fixed_faces[2] = {U, D};
    static const CUBE_FACE rotate_side_faces[4] = {L, F, R, B};
    static const CUBE_FACE roll_fixed_faces[2] = {L, R};
    static const CUBE_FACE roll_side_faces[4] = {B, D, F, U};

    const CUBE_FACE (&fixed_faces)[2] = (dir == ROTATE)? rotate_fixed_faces: roll_fixed_faces;
    const CUBE_FACE (&side_faces)[4] = (dir == ROTATE)? rotate_side_faces: roll_side_faces;

    RotateFace(fixed_faces[0], CW, true);

    char tmp_face[piece_num_];
    char tmp_face_mapping;
    std::memcpy(tmp_face, &(faces_[side_faces[0] * piece_num_]), piece_num_);
    tmp_face_mapping = face_mappings_[side_faces[0]];
    for (int i = 0; i < 3; i ++) {
        std::memcpy(&(faces_[side_faces[i] * piece_num_]), &(faces_[side_faces[i + 1] * piece_num_]), piece_num_);
        face_mappings_[side_faces[i]] = face_mappings_[side_faces[i + 1]];
    }
    std::memcpy(&(faces_[side_faces[3] * piece_num_]), tmp_face, piece_num_);
    face_mappings_[side_faces[3]] = tmp_face_mapping;

    if (dir == ROLL) {
        for (int i = 0; i < 2; i ++) {
            RotateFace(U, CW, true);
            RotateFace(B, CW, true);
        }
    }

    RotateFace(fixed_faces[1], CCW, true);
}


void RubikCube::RotateFace(const CUBE_FACE& rot_face, const ROTATE_DIR& dir, const bool& face_only/* = false*/) {
    assert(rot_face < UNKNOWN_FACE);

    char *face = &(faces_[rot_face * piece_num_]);
    char tmp_face[piece_num_];
    int row = (dir == CW)? 0: (dim_ - 1);
    int col = (dir == CW)? (dim_ - 1): 0;
    int row_step = (dir == CW)? 1: -1;
    int col_step = (dir == CW)? -1: 1;

    for (int r = 0; r < dim_; r ++) {
        for (int c = 0; c < dim_; c ++) {
            tmp_face[(row + (row_step * c)) * dim_ + (col + (col_step * r))] = face[r * dim_ + c];
        }
    }
    std::memcpy(face, tmp_face, piece_num_);

    if (!face_only) {
        int slice_info_idx = GetSliceInfoIndex(rot_face);
        int slice_offset = (rot_face == F || rot_face == R || rot_face == D)? (dim_ - 1): 0;
        ROTATE_DIR slice_dir = (ROTATE_DIR)((rot_face == L || rot_face == D || rot_face == B)? (CCW - dir): dir);

        DoRotateSlice(slice_info_idx, slice_dir, slice_offset);
    }
}


void RubikCube::RotateSlice(const CUBE_SLICE& rot_slice, const ROTATE_DIR& dir, const int &offset/* = 0*/) {
    int slice_info_idx = GetSliceInfoIndex(rot_slice);
    ROTATE_DIR slice_dir = (ROTATE_DIR)((rot_slice == l || rot_slice == d || rot_slice == b)? (CCW - dir): dir);
    int slice_offset = 0;
    switch (rot_slice) {
        case b: case l: case u:
            slice_offset = 1;
            break;
        case X: case Y: case Z:
            slice_offset = dim_ >> 1;
            break;
        case f: case r: case d:
            slice_offset = dim_ - 2;
            break;
        default:
            assert(0);
    }

    DoRotateSlice(slice_info_idx, slice_dir, slice_offset);
}


void RubikCube::DoRotateSlice(const int& slice_info_idx, const ROTATE_DIR& dir, const int& offset/* = 0*/) {
    const SliceInfo (&si)[4] = slice_info[slice_info_idx];
    int start_idx[4] = {0};
    char *n_faces[4];
    for (int i = 0; i < 4; i ++) {
        n_faces[i] = &(faces_[si[i].face_idx * piece_num_]);
        switch (si[i].start_pos) {
            case UL:
                start_idx[i] = 0 + offset * ((si[i].is_row)? dim_: 1);
                break;
            case UR:
                start_idx[i] = (dim_ - 1) + offset * ((si[i].is_row)? dim_: 1);
                break;
            case DL:
                start_idx[i] = dim_ * (dim_ - 1) - offset * ((si[i].is_row)? dim_: 1);
                break;
            case DR:
                start_idx[i] = (piece_num_ - 1) - offset * ((si[i].is_row)? dim_: 1);
                break;
            default:
                assert(0);
        }
    }

    for (int i = 0; i < dim_; i ++) {
        int ni = 4;
        int ni_step = (dir == CW)? 1: -1;
        char tmp_char = n_faces[0][start_idx[0] + (i * si[0].dir * ((si[0].is_row)? 1: dim_))];

        for (int j = 1; j < 4; j ++) {
            int next_ni = ni + ni_step;
            const SliceInfo &curr_s = si[ni % 4];
            const SliceInfo &next_s = si[next_ni % 4];
            n_faces[ni % 4][start_idx[ni % 4] + (i * curr_s.dir * ((curr_s.is_row)? 1: dim_))] =
                n_faces[next_ni % 4][start_idx[next_ni % 4] + (i * next_s.dir * ((next_s.is_row)? 1: dim_))];
            ni += ni_step;
        }

        const SliceInfo &curr_s = si[ni % 4];
        n_faces[ni % 4][start_idx[ni % 4] + (i * curr_s.dir * ((curr_s.is_row)? 1: dim_))] = tmp_char;
    }
}


std::string RubikCube::Scramble(const int& move_count/* = 20*/) {
    const int move_faces_num = (dim_ == 3)? 6: 12;
    std::string ret_moves;

    std::srand(std::clock());

    for (int i = 0; i < move_count; i ++) {
        int move_char_idx = std::rand() % move_faces_num;
        ROTATE_DIR rot_dir = (ROTATE_DIR)(std::rand() % 2);
        if (i > 0)
            ret_moves += ' ';
        ret_moves += move_chars[move_char_idx];
        if (rot_dir == CCW)
            ret_moves += '\'';
    }

    Move(ret_moves);

    return ret_moves;
}


void RubikCube::Move(const std::string& moves) {
    for (int i = 0; i < moves.length(); i ++)
    {
        if (moves[i] == ' ' || moves[i] == '\'' || moves[i] == 'i' || moves[i] == '2')
            continue;

        int move_char_idx = StrChrIdx(move_chars, moves[i], strlen(move_chars));
        assert(move_char_idx >= 0);

        int move_cnt = 1;
        ROTATE_DIR rot_dir = CW;
        // peek next char
        if ((i + 1) < moves.length())
        {
            if (moves[i + 1] == '\'' || moves[i + 1] == 'i')
                rot_dir = CCW;
            else if (moves[i + 1] == '2')
                move_cnt = 2;
        }

        while (move_cnt--) {
            if (move_char_idx < UNKNOWN_FACE)
                RotateFace((CUBE_FACE)move_char_idx, rot_dir);
            else
                RotateSlice((CUBE_SLICE)move_char_idx, rot_dir);
        }
    }
}


void RubikCube::Inverse(const std::string& moves) {
    for (int i = (moves.length() - 1); i >= 0 ; i --)
    {
        if (moves[i] == ' ' || moves[i] == '\'' || moves[i] == 'i' || moves[i] == '2')
            continue;

        int move_char_idx = StrChrIdx(move_chars, moves[i], strlen(move_chars));
        assert(move_char_idx >= 0);

        int move_cnt = 1;
        ROTATE_DIR rot_dir = CCW;
        // peek next char
        if ((i + 1) < moves.length())
        {
            if (moves[i + 1] == '\'' || moves[i + 1] == 'i')
                rot_dir = CW;
            else if (moves[i + 1] == '2')
                move_cnt = 2;
        }

        while (move_cnt--) {
            if (move_char_idx < UNKNOWN_FACE)
                RotateFace((CUBE_FACE)move_char_idx, rot_dir);
            else
                RotateSlice((CUBE_SLICE)move_char_idx, rot_dir);
        }
    }
}


std::string RubikCube::CompressMoves(const std::string& moves) {
    std::string prev_moves = moves;
    std::string comp_moves = CompressMovesImpl(prev_moves);
    while (comp_moves.length() < prev_moves.length())
    {
        prev_moves = comp_moves;
        comp_moves = CompressMovesImpl(prev_moves);
    }
    return comp_moves;
}


std::string RubikCube::CompressMovesImpl(const std::string& moves) {
    std::string compressed_moves;

    int last_move_char_idx = -1;
    ROTATE_DIR last_rot_dir;
    int last_rot_count = 0;

    for (int i = 0; i < moves.length(); i ++)
    {
        if (moves[i] == ' ' || moves[i] == '\'' || moves[i] == 'i' || moves[i] == '2')
            continue;

        int move_char_idx = StrChrIdx(move_chars, moves[i], strlen(move_chars));
        assert(move_char_idx >= 0);

        ROTATE_DIR rot_dir = CW;
        int rot_count = 1;
        // peek next char
        if ((i + 1) < moves.length())
        {
            if (moves[i + 1] == '\'' || moves[i + 1] == 'i')
                rot_dir = CCW;
            else if (moves[i + 1] == '2')
                rot_count = 2;
        }

        if (last_move_char_idx == move_char_idx) {
            last_rot_count += rot_count;
            assert(last_rot_count >= 2 && last_rot_count <= 4);
            if (last_rot_count == 3) {
                if (rot_count == 2)
                    last_rot_dir = (last_rot_dir == CW)? CCW: CW;
                else
                    last_rot_dir = (rot_dir == CW)? CCW: CW;
                last_rot_count = 1;
            } else if ((last_rot_count == 2 && last_rot_dir != rot_dir) ||
                       (last_rot_count == 4)) {
                last_move_char_idx = -1;
            }
        } else {
            if (last_move_char_idx != -1) {
                if (compressed_moves.length() > 0)
                    compressed_moves += ' ';
                compressed_moves += move_chars[last_move_char_idx];
                if (last_rot_count == 2)
                    compressed_moves += '2';
                else if (last_rot_dir == CCW)
                    compressed_moves += '\'';
            }

            last_move_char_idx = move_char_idx;
            last_rot_dir = rot_dir;
            last_rot_count = rot_count;
        }
    }

    if (last_move_char_idx != -1) {
        if (compressed_moves.length() > 0)
            compressed_moves += ' ';
        compressed_moves += move_chars[last_move_char_idx];
        if (last_rot_count == 2)
            compressed_moves += '2';
        else if (last_rot_dir == CCW)
            compressed_moves += '\'';
    }

    return compressed_moves;
}


std::string RubikCube::GetCubeString(const bool& is_color/* = false*/) {
    char tmp_faces[std::strlen(faces_)];
    std::strcpy(tmp_faces, faces_);
    if (is_color)
        for (int i = 0; i < std::strlen(tmp_faces); i ++)
            tmp_faces[i] = FaceCharToColor(tmp_faces[i]);
    return std::string(tmp_faces);
}


bool RubikCube::IsSolved() {
    for (int i = 0; i < face_num; i ++) {
        const char *face = &(faces_[piece_num_ * i]);
        const char face_char = face[0];
        for (int j = 1; j < piece_num_; j ++)
            if (face_char != face[j])
                return false;
    }
    return true;
}


char RubikCube::GetMappedFaceChar(const CUBE_FACE& cube_face) {
    return face_mappings_[cube_face];
}
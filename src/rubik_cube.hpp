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
#pragma once

#include <string>
#include <cstring>
#include <cassert>


namespace rb {

enum CUBE_FACE {
    U = 0,
    L,
    F,
    R,
    B,
    D,
    UNKNOWN_FACE
};

enum CUBE_SLICE {
    u = 6,
    l,
    f,
    r,
    b,
    d,
    X,
    Y,
    Z,
    UNKNOWN_SLICE
};

enum ROTATE_DIR {
    CW = 0,
    CCW
};

enum ROTATE_CUBE_DIR {
    ROTATE = 0,
    ROLL,
};

enum FACE_EDGE {
    LE = 0,
    DE,
    RE,
    UE,
};


static const char* move_chars = "ULFRBDulfrbdXYZ";


CUBE_FACE CvtFaceCharToFace(const char& face_char);


class RubikCube {
  public:
    RubikCube(int dim = 3);
    RubikCube(const char* colors, int dim = 3);
    ~RubikCube();
    RubikCube(const RubikCube& other);
    RubikCube& operator=(const RubikCube& other);

    bool IsSolved();
    void Dump(const bool& is_color = false);
    std::string GetCubeString(const bool& is_color = false);
    char GetMappedFaceChar(const CUBE_FACE& cube_face);
    char GetPieceChar(const CUBE_FACE& cube_face, const int& row, const int& col, const bool& is_color);
    int GetDim() { return dim_; }

    std::string Scramble(const int& Move_count = 20);
    void Move(const std::string& Moves);
    void Inverse(const std::string& Moves);
    void RotateCube(const ROTATE_CUBE_DIR& dir);

    std::string CompressMoves(const std::string& Moves);

  private:
    void MapColors(const char* colors);

    char ColorToFaceChar(const char& color);
    char FaceCharToColor(const char& face_char);

    void RotateFace(const CUBE_FACE& rot_face, const ROTATE_DIR& dir, const bool& face_only = false);
    void RotateSlice(const CUBE_SLICE& rot_slice, const ROTATE_DIR& dir, const int& offset = 0);
    void DoRotateSlice(const int& slice_info_idx, const ROTATE_DIR& dir, const int& offset = 0);
    std::string CompressMovesImpl(const std::string& Moves);

    int dim_;
    int piece_num_;
    char* faces_;
    char* color_mappings_;
    char* face_mappings_;
};

}
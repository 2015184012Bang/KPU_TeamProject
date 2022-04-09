#pragma once

struct Tag_StaticMesh {};
struct Tag_SkeletalMesh {};
struct Tag_Camera {};
struct Tag_Sprite {};
struct Tag_Text {};
struct Tag_DontDestroyOnLoad {};
struct Tag_Player {};
struct Tag_Enemy {};
struct Tag_Tile {};

// 애니메이션 동기화용
struct Tag_Moved {};


// 서버용
struct Tag_UpdateTransform {};
struct Tag_Dead {};
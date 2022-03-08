
// 岩石壊しゲーム 
//
// oskn は osakana4242 の略 
// 関数プロトタイプ宣言を使わない縛り. (なんとなく見た目がスッキリする様な気がして )

// // SDKDDKVer.h をインクルードすると、利用できる最も高いレベルの Windows プラットフォームが定義されます。
// 以前の Windows プラットフォーム用にアプリケーションをビルドする場合は、WinSDKVer.h をインクルードし、
// サポートしたいプラットフォームに _WIN32_WINNT マクロを設定してから SDKDDKVer.h をインクルードします。
#include <SDKDDKVer.h>

#define WIN32_LEAN_AND_MEAN // Windows ヘッダーからほとんど使用されていない部分を除外する
#include <windows.h>

// C ランタイム ヘッダー ファイル
#include <stdlib.h>
#include <stdbool.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <math.h>
#include <assert.h>

// 型 
typedef float oskn_Angle;

typedef struct _oskn_Time {
	float time;
	int frameCount;
	float deltaTime;
} oskn_Time;

typedef struct _oskn_Vec2 {
	float x;
	float y;
} oskn_Vec2;

typedef struct _oskn_Rect {
	float x;
	float y;
	float width;
	float height;
} oskn_Rect;

typedef struct _oskn_Transform {
	oskn_Vec2 position;
	oskn_Angle rotation;
} oskn_Transform;

typedef struct _oskn_Collider {
	float radius;
} oskn_Collider;

typedef struct _oskn_Rigidbody {
	bool enabled;
	oskn_Vec2 velocity;

	INT32 hitCount;
	/// <summary>1フレーム内で他者と衝突したタイミング. deltaTime の割合で表現. [0..1]</summary>
	float hitT;
	oskn_Vec2 nextVelocity;
	oskn_Vec2 nextPosition;
	/// <summary>他のものにぶつかっても位置補正、ベクトル補正を行わない</summary>
	bool isTrigger;
} oskn_Rigidbody;

typedef struct _oskn_HitInfo {
	float t;
	oskn_Vec2 hitPosition;
	oskn_Vec2 aPosition;
	oskn_Vec2 aVelocity;
	oskn_Vec2 bPosition;
	oskn_Vec2 bVelocity;
} oskn_HitInfo;

typedef enum _oskn_Key {
	OSKN_KEY_NONE = 0,
	OSKN_KEY_START = 1 << 0,
	OSKN_KEY_LEFT = 1 << 1,
	OSKN_KEY_RIGHT = 1 << 2,
	OSKN_KEY_UP = 1 << 3,
	OSKN_KEY_DOWN = 1 << 4,
	OSKN_KEY_SHOT = 1 << 5,
	OSKN_KEY_FIX = 1 << 6,
} oskn_Key;

typedef struct _oskn_Input {
	INT32 keyStatePrev;
	INT32 keyState;
	INT32 keyStateNext;
} oskn_Input;

typedef struct _oskn_ObjId {
	INT32 id;
	INT32 index;
} oskn_ObjId;

typedef struct _oskn_ObjHitInfo {
	oskn_ObjId aId;
	oskn_ObjId bId;
	oskn_Vec2 hitPosition;
} oskn_ObjHitInfo;

typedef struct _oskn_ObjHitInfoList {
	oskn_ObjHitInfo* list;
	INT32 count;
	INT32 capacity;
} oskn_ObjHitInfoList;

typedef struct _oskn_Player {
	float hp;
	float shotInterval1;
	float shotInterval2;
	float shotStartTime;
	/// <summary>ショット1発に必要な燃料量</summary>
	float shotFuelConsume;
	/// <summary>燃料残量</summary>
	float shotFuelRest;
	/// <summary>燃料回復量(秒間)</summary>
	float shotFuelRecover;
	/// <summary>自動回復で回復できる燃料上限</summary>
	float shotFuelCapacity1;
	float shotFuelCapacity2;
} oskn_Player;

typedef struct _oskn_DirectionMarker {
	oskn_ObjId ownerId;
} oskn_DirectionMarker;

typedef struct _oskn_Bullet {
	/// <summary>レベル. 色が変わるだけ</summary>
	UINT8 lv;
	float damage;
	float speed;
} oskn_Bullet;

typedef struct _oskn_Enemy {
	/// <summary>1,2,3</summary>
	UINT8 lv;
	float hp;
	float speed;
} oskn_Enemy;

typedef struct _oskn_Camera {
	INT32 playerId;

} oskn_Camera;

typedef enum _oskn_ObjType {
	oskn_ObjType_None,
	oskn_ObjType_Camera,
	oskn_ObjType_Player,
	oskn_ObjType_DirectionMarker,
	oskn_ObjType_Enemy,
	oskn_ObjType_Fuel,
	oskn_ObjType_PlayerBullet,
	oskn_ObjType_EnemyBullet,
} oskn_ObjType;

typedef struct _oskn_Obj {
	bool destroyed;
	/// <summary>この時間に達していたら remove する. 0 以下は無効.</summary>
	float destroyTime;
	oskn_ObjId id;
	float spawnedTime;

	oskn_ObjType type;
	oskn_Transform transform;
	oskn_Collider collider;
	oskn_Rigidbody rigidbody;
	oskn_Player player;
	oskn_DirectionMarker directionMarker;
	oskn_Bullet bullet;
	oskn_Enemy enemy;
} oskn_Obj;

typedef struct _oskn_ObjList {
	oskn_Obj* totalList;
	oskn_ObjId* activeIdList;
	INT32 activeIdListCount;
	INT32 capacity;
	INT32 nextId;
} oskn_ObjList;

typedef enum _oskn_AppState {
	oskn_AppState_None,
	oskn_AppState_Title,
	oskn_AppState_Ready,
	oskn_AppState_Main,
	oskn_AppState_Clear,
	oskn_AppState_Over,
} oskn_AppState;

typedef struct _oskn_App {
	oskn_ObjList objList;
	/// <summary>1フレーム前の状態をまるっと保存</summary>
	oskn_ObjList prevObjList;

	oskn_ObjHitInfoList hitInfoList;

	oskn_ObjId playerId;
	oskn_ObjId cameraId;
	float fps;
	float frameInterval;
	oskn_Vec2 screenSize;
	oskn_Rect areaRect;
	oskn_Time time;
	HBITMAP hBitmap;
	HDC     hdcMem;
	oskn_Input input;
	INT32 enemyAddCountMax;
	INT32 enemyAddCount;

	oskn_AppState appState;
	float appStateStartTime;
	
} oskn_App;


// 定数、グローバル変数 

/// <summary>衝突検証モード. プレイヤー無敵, ステージを狭める. 岩の配置を固定.</summary>
bool OSKN_COL_TEST_ENABLED = true;
bool OSKN_COL_POS_ADJUST_DELAY_ENABLED = false;
/// <summary>衝突時に位置を補正する.</summary>
bool OSKN_COL_POS_ADJUST_ENABLED = true;

#define OSKN_ANGLE_PI 3.141592f
#define OSKN_DEG_TO_RAD (OSKN_ANGLE_PI / 180.0f)
#define OSKN_DEG_TO_RAD (OSKN_ANGLE_PI / 180.0f)
#define OSKN_RAD_TO_DEG (180.0f / OSKN_ANGLE_PI)

HINSTANCE hInst_g = NULL;
oskn_App app_g = { 0 };


// 関数 

float oskn_Math_abs(float v) {
	return (v < 0) ? -v : v;
}

float oskn_Math_clamp(float v, float vMin, float vMax) {
	return (v < vMin) ? vMin :
	                    (vMax < v) ? vMax :
	                                 v;
}

float oskn_Math_lerpUnclamped(float a, float b, float t) {
	return a + (b - a) * t;
}

bool oskn_Float_roundEq(float a, float b, float threshold) {
	float d = a - b;
	return -threshold <= d && d <= threshold;
}

float oskn_Float_moveTowards(float current, float target, float maxDelta) {
	float delta = target - current;
	if (0.0f < delta) {
		delta = min(maxDelta, delta);
	}
	else {
		delta = max(-maxDelta, delta);
	}
	current += delta;
	return current;
}

float oskn_Angle_toRad(oskn_Angle self) {
	return (self) * OSKN_DEG_TO_RAD;
}

float oskn_Angle_toDeg(oskn_Angle self) {
	return self;
}

oskn_Angle oskn_AngleUtil_fromRad(float rad) {
	return rad * OSKN_RAD_TO_DEG;
}


void oskn_Time_add(oskn_Time* self, float deltaTime) {
	self->time += deltaTime;
	self->deltaTime = deltaTime;
	++self->frameCount;
}


bool oskn_Vec2_eq(oskn_Vec2 a, oskn_Vec2 b) {
	return a.x == b.x && a.y == b.y;
}

bool oskn_Vec2_roundEq(oskn_Vec2 a, oskn_Vec2 b, float threshold) {
	return
		oskn_Float_roundEq(a.x, b.x, threshold) &&
		oskn_Float_roundEq(a.y, b.y, threshold);
}

bool oskn_Vec2_isZero(oskn_Vec2 self) {
	return self.x == 0.0f && self.y == 0.0f;
}

float oskn_Vec2_sqrMagnitude(oskn_Vec2 self) {
	return self.x * self.x + self.y * self.y;
}

float oskn_Vec2_magnitude(oskn_Vec2 self) {
	return sqrtf(oskn_Vec2_sqrMagnitude(self));
}

oskn_Vec2 oskn_Vec2_normalize(oskn_Vec2 self) {
	if (self.x == 0 && self.y == 0) return self;
	float mag = oskn_Vec2_magnitude(self);
	self.x /= mag;
	self.y /= mag;
	return self;
}

oskn_Angle oskn_Vec2_toAngle(oskn_Vec2 self) {
	if (self.x == 0 && self.y == 0) return 0.0f;
	float rad = atan2f(self.y, self.x);
	return oskn_AngleUtil_fromRad(rad);
}

oskn_Vec2 oskn_Vec2Util_create(float x, float y) {
	oskn_Vec2 vec;
	vec.x = x;
	vec.y = y;
	return vec;
}

oskn_Vec2 oskn_Vec2Util_addVec2(oskn_Vec2 a, oskn_Vec2 b) {
	a.x += b.x;
	a.y += b.y;
	return a;
}

oskn_Vec2 oskn_Vec2Util_subVec2(oskn_Vec2 a, oskn_Vec2 b) {
	a.x -= b.x;
	a.y -= b.y;
	return a;
}

oskn_Vec2 oskn_Vec2Util_mulF(oskn_Vec2 a, float f) {
	a.x *= f;
	a.y *= f;
	return a;
}

oskn_Vec2 oskn_Vec2Util_lerpUnclamped(oskn_Vec2 a, oskn_Vec2 b, float t) {
	return oskn_Vec2Util_create(
		oskn_Math_lerpUnclamped(a.x, b.x, t),
		oskn_Math_lerpUnclamped(a.y, b.y, t)
	);
}

float oskn_Vec2Util_dot(oskn_Vec2 a, oskn_Vec2 b) {
	return a.x * b.x + a.y * b.y;
}

oskn_Vec2 oskn_Vec2Util_moveTowards(oskn_Vec2 current, oskn_Vec2 target, float maxDelta) {
	oskn_Vec2 deltaVec = oskn_Vec2Util_subVec2(target, current);
	oskn_Vec2 v = oskn_Vec2_normalize(deltaVec);
	float distance = oskn_Vec2_magnitude(deltaVec);
	float deltaF = (maxDelta < distance) ? maxDelta : distance;
	v = oskn_Vec2Util_mulF(v, deltaF);
	current = oskn_Vec2Util_addVec2(current, v);
	return current;
}

oskn_Vec2 oskn_Vec2Util_fromAngle(oskn_Angle angle) {
	float rad = oskn_Angle_toRad(angle);
	oskn_Vec2 vec;
	vec.x = cosf(rad);
	vec.y = sinf(rad);
	return vec;
}


oskn_Vec2 oskn_Rect_min(oskn_Rect self) {
	return oskn_Vec2Util_create(self.x, self.y);
}

oskn_Vec2 oskn_Rect_max(oskn_Rect self) {
	return oskn_Vec2Util_create(self.x + self.width, self.y + self.height);
}

oskn_Vec2 oskn_Rect_center(oskn_Rect self) {
	return oskn_Vec2Util_create(
		self.x + self.width * 0.5f,
		self.y + self.height * 0.5f
	);
}

oskn_Rect oskn_RectUtil_create(float x, float y, float width, float height) {
	oskn_Rect self;
	self.x = x;
	self.y = y;
	self.width = width;
	self.height = height;
	return self;
}


oskn_Key oskn_Key_fromWParam(WPARAM wParam) {
	switch (wParam) {
	case VK_SPACE:    return OSKN_KEY_START;

	case VK_LEFT:     return OSKN_KEY_LEFT;
	case VK_RIGHT:    return OSKN_KEY_RIGHT;
	case VK_UP:       return OSKN_KEY_UP;
	case VK_DOWN:     return OSKN_KEY_DOWN;
	case 'Z':         return OSKN_KEY_SHOT;
	case VK_SHIFT:    return OSKN_KEY_FIX;

	case 'A':         return OSKN_KEY_LEFT;
	case 'D':         return OSKN_KEY_RIGHT;
	case 'W':         return OSKN_KEY_UP;
	case 'S':         return OSKN_KEY_DOWN;
	case 'J':         return OSKN_KEY_SHOT;

	default:          return OSKN_KEY_NONE;
	}
}


bool oskn_Input_hasKey(const oskn_Input* self, oskn_Key key) {
	return 0 != (self->keyState & key);
}

bool oskn_Input_hasKeyDown(const oskn_Input* self, oskn_Key key) {
	return
		(0 == (self->keyStatePrev & key)) &&
		(0 != (self->keyState & key));
}

bool oskn_Input_hasKeyUp(const oskn_Input* self, oskn_Key key) {
	return
		(0 != (self->keyStatePrev & key)) &&
		(0 == (self->keyState & key));
}

oskn_Vec2 oskn_Input_getDirection(const oskn_Input* self) {
	oskn_Vec2 vec = oskn_Vec2Util_create(0.0f, 0.0f);
	if (oskn_Input_hasKey(self, OSKN_KEY_LEFT)) {
		vec.x = -1.0f;
	} else if (oskn_Input_hasKey(self, OSKN_KEY_RIGHT)) {
		vec.x = 1.0f;
	}

	if (oskn_Input_hasKey(self, OSKN_KEY_UP)) {
		vec.y = -1.0f;
	} else if (oskn_Input_hasKey(self, OSKN_KEY_DOWN)) {
		vec.y = 1.0f;
	}

	if (vec.x != 0.0f && vec.y != 0.0f) {
		vec = oskn_Vec2_normalize(vec);
	}

	return vec;
}

void oskn_Input_update(oskn_Input* self) {
	self->keyStatePrev = self->keyState;
	self->keyState = self->keyStateNext;
}


bool oskn_ObjId_eq(oskn_ObjId a, oskn_ObjId b) {
	return a.id == b.id;
}


bool oskn_Obj_isNeedHitTest(const oskn_Obj* self, const oskn_Obj* other) {
	oskn_ObjType type1 = min(self->type, other->type);
	oskn_ObjType type2 = max(self->type, other->type);

	switch (type1) {
	case oskn_ObjType_Player:
		switch (type2) {
		case oskn_ObjType_Fuel:
		case oskn_ObjType_Enemy:
		case oskn_ObjType_EnemyBullet:
			if (OSKN_COL_TEST_ENABLED) {
				return false;
			}
			return true;
		default: return false;
		}
	case oskn_ObjType_Enemy:
		switch (type2) {
		case oskn_ObjType_Enemy:
		case oskn_ObjType_Player:
		case oskn_ObjType_PlayerBullet:
			return true;
		default: return false;
		}
	case oskn_ObjType_PlayerBullet:
		switch (type2) {
		case oskn_ObjType_Enemy:
			return true;
		default: return false;
		}
	case oskn_ObjType_EnemyBullet:
		switch (type2) {
		case oskn_ObjType_Player:
			return true;
		default: return false;
		}
	default: return false;
	}
}

bool oskn_ObjList_init(oskn_ObjList* self, int capacity) {
	self->capacity = capacity;
	self->totalList = calloc(self->capacity, sizeof(*self->totalList));
	self->activeIdList = calloc(self->capacity, sizeof(*self->activeIdList));
	for (int i = 1, iCount = self->capacity; i < iCount; ++i) {
		oskn_Obj* item = &self->totalList[i];
		item->destroyed = true;
	}

	return true;
}

bool oskn_ObjList_copyFrom(oskn_ObjList* self, const oskn_ObjList* other) {
	if (self->capacity != other->capacity) {
		assert("unko");
		return false;
	}
	for (int i = 0, iCount = other->capacity; i < iCount; ++i) {
		self->totalList[i] = other->totalList[i];
	}
	for (int i = 0, iCount = other->activeIdListCount; i < iCount; ++i) {
		self->activeIdList[i] = other->activeIdList[i];
	}
	self->nextId = other->nextId;
	return true;
}

bool oskn_ObjList_free(oskn_ObjList* self) {
	free(self->activeIdList);
	self->activeIdList = NULL;
	free(self->totalList);
	self->totalList = NULL;
	return true;
}

oskn_Obj* oskn_ObjList_getByIndex(const oskn_ObjList* self, INT32 index) {
	if (index < 0 || self->capacity <= index) return NULL;
	return &self->totalList[index];
}

/// <summary>取得できなかったら false</summary>
bool oskn_ObjList_tryGetById(const oskn_ObjList* self, oskn_ObjId id, oskn_Obj** obj) {
	*obj = NULL;
	if (id.id == 0) return false;
	*obj = oskn_ObjList_getByIndex(self, id.index);
	if (NULL == obj) return false;
	if (!oskn_ObjId_eq((*obj)->id, id)) return false;
	return true;
}

/// <summary>取得できなかったら assert 後に false. 必ず取得できるつもりのときはコチラを使う</summary>
bool oskn_ObjList_getById(const oskn_ObjList* self, oskn_ObjId id, oskn_Obj** obj) {
	if (!oskn_ObjList_tryGetById(self, id, obj)) {
		assert(false);
		return false;
	}
	return true;
}

oskn_Obj* oskn_ObjList_add(oskn_ObjList* self) {
	int i = 1;
	oskn_Obj* target = NULL;
	for (int iCount = self->capacity; i < iCount; ++i) {
		oskn_Obj* item = &self->totalList[i];
		if (item->destroyed) {
			target = item;
			break;
		}
	}

	if (NULL == target) return 0;

	++self->nextId;
	oskn_ObjId objId = { 0 };

	objId.id = self->nextId;
	objId.index = i;
	memset(target, 0, sizeof(*target));
	target->destroyed = false;
	target->destroyTime = 0.0f;
	target->id = objId;
	target->spawnedTime = app_g.time.time;

	self->activeIdList[self->activeIdListCount] = objId;
	++self->activeIdListCount;

	return target;
}

bool oskn_ObjList_requestRemoveById(oskn_ObjList* self, oskn_ObjId id, float time) {
	oskn_Obj* obj = NULL;
	if (!oskn_ObjList_tryGetById(self, id, &obj)) return false;
	obj->destroyTime = app_g.time.time + time;
	return true;
}

bool oskn_ObjList_removeById(oskn_ObjList* self, oskn_ObjId id) {
	for (int i = self->activeIdListCount - 1; 0 <= i; --i) {
		oskn_ObjId id2 = self->activeIdList[i];
		if (!oskn_ObjId_eq(id2, id)) continue;

		for (int j = i, jCount = self->activeIdListCount; j < jCount; ++j) {
			self->activeIdList[j] = self->activeIdList[j + 1];
		}
		--self->activeIdListCount;

		oskn_Obj* obj = NULL;
		if (!oskn_ObjList_getById(self, id, &obj)) continue;
		memset(obj, 0, sizeof(*obj));
		obj->destroyed = true;
		return true;
	}
	return false;
}

bool oskn_ObjList_clear(oskn_ObjList* self) {
	for (int i = self->activeIdListCount - 1; 0 <= i; --i) {
		oskn_ObjId objId = self->activeIdList[i];
		oskn_ObjList_removeById(self, objId);
	}
	self->nextId = 0;
	return true;
}


bool oskn_ObjHitInfoList_init(oskn_ObjHitInfoList* self, int capacity) {
	self->count = 0;
	self->capacity = capacity;
	self->list= calloc(self->capacity, sizeof(*self->list));
	return true;
}

bool oskn_ObjHitInfoList_free(oskn_ObjHitInfoList* self) {
	free(self->list);
	self->list = NULL;
	return true;
}

void oskn_ObjHitInfoList_clear(oskn_ObjHitInfoList* self) {
	self->count = 0;
}

bool oskn_ObjHitInfoList_add(oskn_ObjHitInfoList* self, oskn_ObjHitInfo item) {
	if (self->capacity < self->count) {
		assert(false);
		return false;
	}
	self->list[self->count] = item;
	++self->count;
	return true;
}

oskn_ObjHitInfo oskn_ObjHitInfoList_get(oskn_ObjHitInfoList* self, int index) {
	if (index < 0 || self->count <= index) {
		assert(false);
		oskn_ObjHitInfo item = { 0 };
		return item;
	}
	return self->list[index];
}



oskn_Obj* oskn_App_createEnemy(oskn_App* self, oskn_Vec2 pos, UINT8 lv) {
	oskn_Obj* obj = oskn_ObjList_add(&app_g.objList);
	obj->type = oskn_ObjType_Enemy;
	obj->transform.position = pos;
	obj->collider.radius = 12.0f * lv;
	obj->enemy.hp = (float)(lv * 4);
	obj->enemy.lv = lv;

	obj->transform.rotation = self->time.time * 10.0f;
	float speed = 25.0f + 75.0f * rand() / RAND_MAX / lv;
	obj->rigidbody.enabled = true;
	obj->rigidbody.velocity = oskn_Vec2Util_mulF(oskn_Vec2Util_fromAngle(obj->transform.rotation), speed);

	return obj;
}

oskn_Vec2 oskn_App_reflectVec(oskn_Vec2 v, oskn_Vec2 n) {
	oskn_Vec2 iv = oskn_Vec2Util_mulF(v, -1);
	float a = oskn_Vec2Util_dot(iv, n);
	//// 壁に平行なベクトル.
	//oskn_Vec2 wallVec = oskn_Vec2Util_addVec2(v, oskn_Vec2Util_mulF(n, a));
	oskn_Vec2 r = oskn_Vec2Util_addVec2(v, oskn_Vec2Util_mulF(n, 2 * a));
	return r;
}

// 円が指定位置に衝突したときの反射ベクトルを得る.
bool oskn_App_tryGetReflectVecByHitPos(oskn_Vec2 pos, oskn_Vec2 velocity, oskn_Vec2 hitPos, oskn_Vec2* outVelocity ) {
	oskn_Vec2 toHitPos = oskn_Vec2Util_subVec2(hitPos, pos);
	oskn_Vec2 toHitPosN = oskn_Vec2_normalize(toHitPos);
	oskn_Vec2 reflectVecN = oskn_Vec2Util_mulF(toHitPosN, -1);
	// oskn_Vec2 reflectVec = oskn_Vec2Util_mulF(reflectVecN, aObj->collider.radius);
	// aPos = oskn_Vec2Util_addVec2(hitPos, reflectVec);
	float dot = oskn_Vec2Util_dot(velocity, toHitPos);
	if (0 < dot) {
		oskn_Vec2 r = oskn_App_reflectVec(velocity, reflectVecN);
		*outVelocity = r;
		return true;
	}
	else {
		*outVelocity = velocity;
		return false;
	}
}

/// <summary>aObj, bObj が衝突した際の位置補正、ベクトル補正.</summary>
bool oskn_App_reflectObj(const oskn_Obj* aObj, const oskn_Obj* bObj, oskn_Vec2 aPrevPos, oskn_Vec2 bPrevPos, const oskn_Time* time, oskn_HitInfo* outHitInfo) {
	oskn_Vec2 aPos = aObj->transform.position;
	oskn_Vec2 bPos = bObj->transform.position;
	oskn_Vec2 aDelta = oskn_Vec2Util_subVec2(aPos, aPrevPos);
	oskn_Vec2 bDelta = oskn_Vec2Util_subVec2(bPos, bPrevPos);

	// t = (d - aPos + bPos) / (-aVec + bVec)
	float hitD = aObj->collider.radius + bObj->collider.radius;
	//float t = (d + oskn_Vec2_magnitude(oskn_Vec2Util_subVec2(bObj->transform.position, aObj->transform.position))) /
	//	oskn_Vec2_magnitude(oskn_Vec2Util_subVec2(bObj->rigidbody.velocity, aObj->rigidbody.velocity));
	//oskn_Vec2 aPrevPos = oskn_Vec2Util_subVec2(aPos, oskn_Vec2Util_mulF(aObj->rigidbody.velocity, time->deltaTime));
	//oskn_Vec2 bPrevPos = oskn_Vec2Util_subVec2(bPos, oskn_Vec2Util_mulF(bObj->rigidbody.velocity, time->deltaTime));

	// x - ((bPos + bVel * t) - (aPos + aVel * t)) = 0
	// x - (bPos + bVel * t - aPos - aVel * t) = 0
	// x - bPos - bVel * t + aPos + aVel * t = 0
	// x - bPos + aPos + (aVel - bVel) * t = 0
	// (aVel - bVel) * t = - x + bPos - aPos
	// t = (- x + bPos - aPos) / (aVel - bVel)
	float t = 0.0f;
	float vel = oskn_Vec2_magnitude(oskn_Vec2Util_subVec2(aDelta, bDelta));
	float d2 = oskn_Vec2_magnitude(oskn_Vec2Util_subVec2(bPrevPos, aPrevPos));
	t = (vel < 0.001f) ?
		0.0f :
		(d2 - hitD) / vel;

	if (1.0f < t) {
		// 当たってない.
		return false;
	}

	oskn_Vec2 hitPos;
	if (t <= 0) {
		// お互い離れようとしてる.
		// はじめからめり込んでる.
		hitPos = oskn_Vec2Util_lerpUnclamped(aPos, bPos, 0.5f);
	}
	else {
		aPos = oskn_Vec2Util_addVec2(aPrevPos, oskn_Vec2Util_mulF(aDelta, t));
		bPos = oskn_Vec2Util_addVec2(bPrevPos, oskn_Vec2Util_mulF(bDelta, t));
	}
	hitPos = oskn_Vec2Util_lerpUnclamped(aPos, bPos, aObj->collider.radius / hitD);

	outHitInfo->hitPosition = hitPos;
	outHitInfo->t = t;

	oskn_Vec2 reflectVec;

	// a の位置補正, ベクトル補正.
	oskn_App_tryGetReflectVecByHitPos(aPos, aObj->rigidbody.velocity, hitPos, &reflectVec);
	outHitInfo->aVelocity = reflectVec;
	outHitInfo->aPosition = OSKN_COL_POS_ADJUST_ENABLED ? aPos : aObj->transform.position;

	// b の位置補正, ベクトル補正.
	oskn_App_tryGetReflectVecByHitPos(bPos, bObj->rigidbody.velocity, hitPos, &reflectVec);
	outHitInfo->bVelocity = reflectVec;
	outHitInfo->bPosition = OSKN_COL_POS_ADJUST_ENABLED ? bPos : bObj->transform.position;

	return true;
}

bool testReflect2(const oskn_Obj* aObj, const oskn_Obj* bObj, float deltaTime, oskn_HitInfo* outHitInfo) {
	oskn_Vec2 aPos = aObj->transform.position;
	oskn_Collider aCol = aObj->collider;
	oskn_Vec2 bPos = bObj->transform.position;
	oskn_Collider bCol = bObj->collider;

	float radius = aCol.radius + bCol.radius;
	float sqrRadius = radius * radius;
	oskn_Vec2 aToB = oskn_Vec2Util_subVec2(bPos, aPos);
	float sqrDistance = oskn_Vec2_sqrMagnitude(aToB);
	bool isHit = sqrDistance < sqrRadius;
	if (!isHit) return false;
	oskn_Time time;
	time.deltaTime = deltaTime;
	oskn_Vec2 aPrevPos = oskn_Vec2Util_subVec2(aPos, oskn_Vec2Util_mulF(aObj->rigidbody.velocity, time.deltaTime));
	oskn_Vec2 bPrevPos = oskn_Vec2Util_subVec2(bPos, oskn_Vec2Util_mulF(bObj->rigidbody.velocity, time.deltaTime));
	oskn_App_reflectObj(aObj, bObj, aPrevPos, bPrevPos, &time, outHitInfo);
	return true;
}

void testReflect1() {
	if (!OSKN_COL_POS_ADJUST_ENABLED) return;
	// 静止してる a に b が突っ込むパターン.
	// 1. b が半分めり込む:
	// 対処: 両者が重なる直前の位置に巻き戻す + 移動ベクトルを反射する.
	//  x: 012345678901234567890123456789
	// a1:    [a1]
	// b1:         [b1]
	// a2:    [a2]
	// b2:     [b2]
	// a3:    [a3]
	// b3:        [b3]
	//
	// 2. b が半径以上めり込む.
	// 対処: 両者が重なる直前の位置に巻き戻す + 移動ベクトルを反射する.
	//  x: 012345678901234567890123456789
	// a1:    [a1]
	// b1:         [b1]
	// a2:    [a2]
	// b2:   [b2]
	// a3:    [a3]
	// b3:        [b3]
	//
	// 3. すでにめり込んでて、動きが無い
	// 対処: なにもしない
	//  x: 012345678901234567890123456789
	// a1:    [a1]
	// b1:     [b1]
	// a2:    [a2]
	// b2:     [b2]
	// a3:    [a3]
	// b3:     [b3]
	//
	// 4. b がすでにめり込んでて、両者の中点側に進もうとしてる:
	// 対処案1: 反対側に位置補正、ベクトルも反射する
	// 対処案2: ベクトルを反射する
	//  x: 012345678901234567890123456789
	// a1:    [a1]
	// b1:     [b1]
	// a2:    [a2]
	// b2:    [b2]
	// a3:    [a3]
	// b3:        [b3]
	//
	// 5. b がすでにめり込んでて、両者の中点の反対側に進もうとしてる:
	// 案1: 進もうとしてる方向に位置補正
	// 案2: ベクトル補正しない
	//  x: 012345678901234567890123456789
	// a1:    [a1]
	// b1:     [b1]
	// a2:    [a2]
	// b2:      [b2]
	// a3:    [a3]
	// b3:        [b3]

	oskn_Obj aObj = { 0 };
	oskn_Obj bObj = { 0 };
	oskn_HitInfo hitInfo = { 0 };

	// 1.
	aObj.collider.radius = 2;
	aObj.transform.position = oskn_Vec2Util_create(5, 0);
	aObj.rigidbody.velocity = oskn_Vec2Util_create(0, 0);
	bObj.collider.radius = 2;
	bObj.transform.position = oskn_Vec2Util_create(7, 0);
	bObj.rigidbody.velocity = oskn_Vec2Util_create(-3, 0);
	assert(testReflect2(&aObj, &bObj, 1.0f, &hitInfo));
	assert(hitInfo.hitPosition.x == 7.0f);
	assert(hitInfo.bPosition.x == 9.0f);
	assert(hitInfo.bVelocity.x == 3.0f);

	// 2.
	aObj.collider.radius = 2;
	aObj.transform.position = oskn_Vec2Util_create(5, 0);
	aObj.rigidbody.velocity = oskn_Vec2Util_create(0, 0);
	bObj.collider.radius = 2;
	bObj.transform.position = oskn_Vec2Util_create(4, 0);
	bObj.rigidbody.velocity = oskn_Vec2Util_create(-6, 0);
	assert(testReflect2(&aObj, &bObj, 1.0f, &hitInfo));
	assert(hitInfo.hitPosition.x == 7.0f);
	assert(hitInfo.bPosition.x == 9.0f);
	assert(hitInfo.bVelocity.x == 6.0f);

	// 3.
	aObj.collider.radius = 2;
	aObj.transform.position = oskn_Vec2Util_create(5, 0);
	aObj.rigidbody.velocity = oskn_Vec2Util_create(0, 0);
	bObj.collider.radius = 2;
	bObj.transform.position = oskn_Vec2Util_create(6, 0);
	bObj.rigidbody.velocity = oskn_Vec2Util_create(0, 0);
	assert(testReflect2(&aObj, &bObj, 1.0f, &hitInfo));
	assert(hitInfo.hitPosition.x == 5.5f);
	assert(hitInfo.bPosition.x == 6.0f);
	assert(hitInfo.bVelocity.x == 0.0f);

	// 4.
	aObj.collider.radius = 2;
	aObj.transform.position = oskn_Vec2Util_create(5, 0);
	aObj.rigidbody.velocity = oskn_Vec2Util_create(0, 0);
	bObj.collider.radius = 2;
	bObj.transform.position = oskn_Vec2Util_create(6, 0);
	bObj.rigidbody.velocity = oskn_Vec2Util_create(-1, 0);
	assert(testReflect2(&aObj, &bObj, 1.0f, &hitInfo));
	assert(hitInfo.hitPosition.x == 5.5f);
	assert(hitInfo.bPosition.x == 6.0f);
	assert(hitInfo.bVelocity.x == 1.0f);

	// 5.
	aObj.collider.radius = 2;
	aObj.transform.position = oskn_Vec2Util_create(5, 0);
	aObj.rigidbody.velocity = oskn_Vec2Util_create(0, 0);
	bObj.collider.radius = 2;
	bObj.transform.position = oskn_Vec2Util_create(6, 0);
	bObj.rigidbody.velocity = oskn_Vec2Util_create(1, 0);
	assert(testReflect2(&aObj, &bObj, 1.0f, &hitInfo));
	assert(hitInfo.hitPosition.x == 5.5f);
	assert(hitInfo.bPosition.x == 6.0f);
	assert(hitInfo.bVelocity.x == 1.0f);
}

void oskn_App_test(oskn_App* self) {
	{
		oskn_Vec2 expected;
		oskn_Vec2 actual;

		expected = oskn_Vec2Util_create(1.0f, 0.0f);
		actual = oskn_Vec2Util_fromAngle(0);
		assert(oskn_Vec2_eq(expected, actual));

		expected = oskn_Vec2Util_create(0.0f, 1.0f);
		actual = oskn_Vec2Util_fromAngle(90);
		assert(oskn_Vec2_roundEq(expected, actual, 0.01f));

		expected = oskn_Vec2Util_create(-1.0f, 0.0f);
		actual = oskn_Vec2Util_fromAngle(180);
		assert(oskn_Vec2_roundEq(expected, actual, 0.01f));

		expected = oskn_Vec2Util_create(0.0f, -1.0f);
		actual = oskn_Vec2Util_fromAngle(-90);
		assert(oskn_Vec2_roundEq(expected, actual, 0.01f));
	}
	{
		oskn_Angle expected;
		oskn_Angle actual;

		expected = 0.0f;
		actual = oskn_Vec2_toAngle(oskn_Vec2Util_create(1.0f, 0.0f));
		assert(oskn_Float_roundEq(expected, actual, 0.01f));

		expected = 90.0f;
		actual = oskn_Vec2_toAngle(oskn_Vec2Util_create(0.0f, 1.0f));
		assert(oskn_Float_roundEq(expected, actual, 0.01f));

		expected = 180.0f;
		actual = oskn_Vec2_toAngle(oskn_Vec2Util_create(-1.0f, 0.0f));
		assert(oskn_Float_roundEq(expected, actual, 0.01f));

		expected = -90.0f;
		actual = oskn_Vec2_toAngle(oskn_Vec2Util_create(0.0f, -1.0f));
		assert(oskn_Float_roundEq(expected, actual, 0.01f));
	}
	{
		oskn_Vec2 expected;
		oskn_Vec2 actual;

		expected = oskn_Vec2Util_create(-1, 1);
		actual = oskn_App_reflectVec(oskn_Vec2Util_create(1, 1), oskn_Vec2Util_create(-1, 0));
		assert(oskn_Vec2_eq(expected, actual));

		expected = oskn_Vec2Util_create(1, -1);
		actual = oskn_App_reflectVec(oskn_Vec2Util_create(-1, -1), oskn_Vec2Util_create(1, 0));
		assert(oskn_Vec2_eq(expected, actual));

		expected = oskn_Vec2Util_create(-2, -2);
		actual = oskn_App_reflectVec(oskn_Vec2Util_create(2, 2), oskn_Vec2_normalize(oskn_Vec2Util_create(-1, -1)));
		assert(oskn_Vec2_roundEq(expected, actual, 0.01f));
	}
	testReflect1();
}

bool oskn_App_init(oskn_App* self, HWND hWnd) {
	oskn_App_test(self);

	oskn_ObjList_init(&self->objList, 255);
	oskn_ObjList_init(&self->prevObjList, self->objList.capacity);
	oskn_ObjHitInfoList_init(&self->hitInfoList, 255);
	self->screenSize.x = 320;
	self->screenSize.y = 240;
	self->fps = 60.0f;
	self->frameInterval = 1.0f / self->fps;
	if (OSKN_COL_TEST_ENABLED) {
		self->areaRect.x = 0;
		self->areaRect.y = 0;
		self->areaRect.width = 150;
		self->areaRect.height = 150;
	} else {
		self->areaRect.x = -64;
		self->areaRect.y = -64;
		self->areaRect.width = 320 * 2 + 128;
		self->areaRect.height = 240 * 2 + 128;
	}
	HDC hdc;
	RECT rc;
	hdc = GetDC(hWnd);                      	// ウインドウのDCを取得
	GetClientRect(GetDesktopWindow(), &rc);  	// デスクトップのサイズを取得
	self->hBitmap = CreateCompatibleBitmap(hdc, rc.right, rc.bottom);
	self->hdcMem = CreateCompatibleDC(NULL);		// カレントスクリーン互換
	SelectObject(self->hdcMem, self->hBitmap);		// MDCにビットマップを割り付け

	self->appState = oskn_AppState_None;
	return true;
}

void oskn_App_onHitObj(oskn_App* self, oskn_Obj* aObj, const oskn_Obj* bObj, oskn_Vec2 hitPos) {
	switch (aObj->type) {
	case oskn_ObjType_Player: {
		switch (bObj->type) {
		case oskn_ObjType_Fuel: {
			float nextFuel = aObj->player.shotFuelRest + aObj->player.shotFuelConsume;
			nextFuel = min(aObj->player.shotFuelCapacity2, nextFuel);
			aObj->player.shotFuelRest = nextFuel;
			break;
		}
		default: {
			if (0 < aObj->player.hp) {
				aObj->player.hp -= 1;
			}
			break;
		}
		}
		break;
	}
	case oskn_ObjType_Fuel: {
		oskn_ObjList_requestRemoveById(&app_g.objList, aObj->id, 0.0f);
		break;
	}
	case oskn_ObjType_PlayerBullet: {
		oskn_ObjList_requestRemoveById(&app_g.objList, aObj->id, 0.0f);
		
		for (int i = 0; i < 4; ++i) {
			oskn_Obj* fuel = oskn_ObjList_add(&app_g.objList);
			fuel->collider.radius = 3.0f;
			fuel->type = oskn_ObjType_Fuel;
			oskn_Vec2 pos = aObj->transform.position;
			oskn_Vec2 vec;
			vec.x = -1.0f + 2.0f * rand() / RAND_MAX;
			vec.y = -1.0f + 2.0f * rand() / RAND_MAX;
			vec = oskn_Vec2_normalize(vec);
			fuel->transform.rotation = oskn_Vec2_toAngle(vec);
			fuel->transform.position = pos;

			float speed = 100.0f + 100.0f * rand() / RAND_MAX;
			oskn_Vec2 vel = oskn_Vec2Util_mulF(vec, speed);
			fuel->rigidbody.enabled = true;
			fuel->rigidbody.velocity = vel;
		}
		break;
	}
	case oskn_ObjType_Enemy: {
		if (bObj->type == oskn_ObjType_Enemy) {
		} else if (bObj->type == oskn_ObjType_PlayerBullet) {
			aObj->enemy.hp -= bObj->bullet.damage;
			if (aObj->enemy.hp <= 0) {
				if (1 < aObj->enemy.lv) {
					for (int i = 0; i < 4; ++i) {
						float radius = aObj->collider.radius;
						oskn_Vec2 pos = aObj->transform.position;
						pos.x += (i % 2) * radius - radius * 0.5f;
						pos.y += (i / 2) * radius - radius * 0.5f;
						oskn_App_createEnemy(&app_g, pos, aObj->enemy.lv - 1);
					}
				}

				oskn_ObjList_requestRemoveById(&app_g.objList, aObj->id, 0.0f);
			}
		}
		break;
	}
	}
}

/// <summary>更新, 移動, 衝突判定</summary>
void oskn_App_updateObj(oskn_App* self) {
	oskn_ObjList* objList = &self->objList;

	for (int i = 0, iCount = objList->activeIdListCount; i < iCount; ++i) {
		oskn_ObjId id = objList->activeIdList[i];
		oskn_Obj* obj = NULL;
		if (!oskn_ObjList_getById(objList, id, &obj)) {
			return;
		}

		switch (obj->type) {
		case oskn_ObjType_Camera: {
			// プレイヤーを追従する.
			oskn_Obj* target = NULL;
			if (oskn_ObjList_tryGetById(&self->objList, self->playerId, &target)) {
				oskn_Vec2 pos = obj->transform.position;
				oskn_Vec2 tpos = target->transform.position;

				// プレイヤーが向いてる方向に画面を広くとる.
				const float cameraTargetOffset = 64.0f;
				tpos = oskn_Vec2Util_addVec2(tpos, oskn_Vec2Util_mulF(oskn_Vec2Util_fromAngle(target->transform.rotation), cameraTargetOffset));
				
				// tpos を画面端で clamp.
				oskn_Vec2 rMin = oskn_Rect_min(app_g.areaRect);
				oskn_Vec2 rMax = oskn_Rect_max(app_g.areaRect);
				rMin = oskn_Vec2Util_addVec2(rMin, oskn_Vec2Util_mulF(app_g.screenSize, 0.5f));
				rMax = oskn_Vec2Util_addVec2(rMax, oskn_Vec2Util_mulF(app_g.screenSize, -0.5f));
				tpos.x = oskn_Math_clamp(tpos.x, rMin.x, rMax.x);
				tpos.y = oskn_Math_clamp(tpos.y, rMin.y, rMax.y);

				// https://osakana4242.hatenablog.com/entry/2021/03/31/230412
				// 秒間 8 割距離を詰める.
				float easeOutSpeed = 0.8f;
				// このフレームで詰める割合.
				float deltaRate = 1.0f - powf(1.0f - easeOutSpeed, self->time.deltaTime);
				oskn_Vec2 mov = oskn_Vec2Util_mulF(oskn_Vec2Util_subVec2(tpos, pos), deltaRate);

				pos = oskn_Vec2Util_addVec2(pos, mov);
				obj->transform.position = pos;
			}
			break;
		}
		case oskn_ObjType_DirectionMarker: {
			// 対象の前方を示す.
			oskn_Obj* target = NULL;
			if (!oskn_ObjList_tryGetById(&self->objList, obj->directionMarker.ownerId, &target)) {
				oskn_ObjList_requestRemoveById(&self->objList, obj->id, 0.0f);
			} else {
				oskn_Vec2 pos = obj->transform.position;
				oskn_Vec2 tpos = target->transform.position;

				// プレイヤーが向いてる方向に画面を広くとる.
				const float cameraTargetOffset = 48.0f;
				tpos = oskn_Vec2Util_addVec2(tpos, oskn_Vec2Util_mulF(oskn_Vec2Util_fromAngle(target->transform.rotation), cameraTargetOffset));

				// https://osakana4242.hatenablog.com/entry/2021/03/31/230412
				// 秒間 9.9 割距離を詰める.
				float easeOutSpeed = 0.99f;
				// このフレームで詰める割合.
				float deltaRate = 1.0f - powf(1.0f - easeOutSpeed, self->time.deltaTime);
				oskn_Vec2 mov = oskn_Vec2Util_mulF(oskn_Vec2Util_subVec2(tpos, pos), deltaRate);

				pos = oskn_Vec2Util_addVec2(pos, mov);
				obj->transform.position = pos;
			}
			break;
		}
		case oskn_ObjType_Player: {
			oskn_Vec2 inputDir = oskn_Input_getDirection(&app_g.input);

			// shot 処理.
			// update 中に追加された Obj はそのフレーム中に update を回すか否か...
			{

				if (oskn_Input_hasKey(&app_g.input, OSKN_KEY_SHOT)) {
					if (obj->player.shotStartTime <= 0.0f) {
						obj->player.shotStartTime = app_g.time.time;
					}
				}
				else {
					obj->player.shotStartTime = 0.0f;
				}

				// 燃料の自動回復.
				if (obj->player.shotFuelRest < obj->player.shotFuelCapacity1) {
					float nextFuel = obj->player.shotFuelRest + obj->player.shotFuelRecover * app_g.time.deltaTime;
					nextFuel = min(obj->player.shotFuelCapacity1, nextFuel);
					obj->player.shotFuelRest = nextFuel;
				}

				if (0.0f < obj->player.shotStartTime) {
					float time = app_g.time.time - obj->player.shotStartTime;
					float prevTime = time - app_g.time.deltaTime;
					// 射撃間隔.
					// 燃料が一定量あると間隔が短くなる.
					INT32 shotLv = (obj->player.shotFuelCapacity1 < obj->player.shotFuelRest) ? 2 : 1;
					float shotInterval = shotLv < 2 ?
						obj->player.shotInterval1 :
						obj->player.shotInterval2;

					INT32 count1 = (time == 0.0f) ? -1 : (INT32)(prevTime / shotInterval);
					INT32 count2 = (INT32)(time / shotInterval);
					if (count1 < count2) {
						float nextFuel = obj->player.shotFuelRest - obj->player.shotFuelConsume;
						bool hasFuel = 0 <= nextFuel;
						if (hasFuel) {
							obj->player.shotFuelRest = nextFuel;

							oskn_Obj* bullet = oskn_ObjList_add(&app_g.objList);
							bullet->type = oskn_ObjType_PlayerBullet;
							bullet->bullet.lv = shotLv;
							bullet->bullet.damage = 1.0f;
							bullet->bullet.speed = 400.0f;
							bullet->collider.radius = 8.0f;
							bullet->transform.position = obj->transform.position;
							bullet->transform.rotation = obj->transform.rotation;
						}
					}
				}
			}


			if (!oskn_Vec2_isZero(inputDir)) {
				oskn_Vec2 pos = obj->transform.position;
				oskn_Vec2 move;
				float speed = 100.0f * app_g.time.deltaTime;
				move.x = inputDir.x * speed;
				move.y = inputDir.y * speed;
				pos.x += move.x;
				pos.y += move.y;


				oskn_Vec2 rectMin = oskn_Rect_min(app_g.areaRect);
				oskn_Vec2 rectMax = oskn_Rect_max(app_g.areaRect);
				if (pos.x - obj->collider.radius < rectMin.x) {
					pos.x = rectMin.x + obj->collider.radius;
				}
				else if (rectMax.x <= pos.x + obj->collider.radius) {
					pos.x = rectMax.x - obj->collider.radius;
				}

				if (pos.y - obj->collider.radius < rectMin.y) {
					pos.y = rectMin.y + obj->collider.radius;
				}
				else if (rectMax.y <= pos.y + obj->collider.radius) {
					pos.y = rectMax.y - obj->collider.radius;
				}

				if (!oskn_Input_hasKey(&app_g.input, OSKN_KEY_FIX)) {
					obj->transform.rotation = oskn_Vec2_toAngle(move);
				}

				obj->transform.position = pos;
			}
			break;
		}
		case oskn_ObjType_PlayerBullet: {
			oskn_Vec2 vec = oskn_Vec2Util_fromAngle(obj->transform.rotation);
			float speed = obj->bullet.speed * app_g.time.deltaTime;
			vec.x *= speed;
			vec.y *= speed;
			oskn_Vec2 pos = obj->transform.position;
			pos.x += vec.x;
			pos.y += vec.y;
			oskn_Vec2 rectMin = oskn_Rect_min(app_g.areaRect);
			oskn_Vec2 rectMax = oskn_Rect_max(app_g.areaRect);

			bool isOutside = false;
			if (pos.x - obj->collider.radius < rectMin.x) {
				isOutside = true;
			}
			else if (rectMax.x <= pos.x + obj->collider.radius) {
				isOutside = true;
			}

			if (pos.y - obj->collider.radius < rectMin.y) {
				isOutside = true;
			}
			else if (rectMax.y <= pos.y + obj->collider.radius) {
				isOutside = true;
			}

			if (isOutside) {
				oskn_ObjList_requestRemoveById(objList, id, 0.0f);
			}

			obj->transform.position = pos;
			obj->transform.rotation = oskn_Vec2_toAngle(vec);
			break;
		}
		case oskn_ObjType_Enemy: {
			break;
		}
		case oskn_ObjType_Fuel: {
			oskn_Obj* player = NULL;
			if (oskn_ObjList_tryGetById(&app_g.objList, app_g.playerId, &player) && 0.0f < player->player.hp) {
				// プレイヤーに吸い込まれる軌道.
				// プレイヤーに近いほど吸い込まれる力が強くなる.
				oskn_Vec2 toTargetVec = oskn_Vec2Util_subVec2(player->transform.position, obj->transform.position);
				float distance = oskn_Vec2_magnitude(toTargetVec);
				float nearDistance = 24.0f;
				float farDistance = 96.0f;
				if (distance < farDistance) {
					distance = max(nearDistance, distance);
					oskn_Vec2 targetVelocity = oskn_Vec2_normalize(toTargetVec);
					{
						// 遠い: 0.0, 近い: 1.0
						float rate = 1.0f - ((distance - nearDistance) / (farDistance - nearDistance));
						// 吸引力.
						float speed = 200.0f * rate;
						targetVelocity = oskn_Vec2Util_mulF(targetVelocity, speed);
					}

					oskn_Vec2 velocity = obj->rigidbody.velocity;
					velocity = oskn_Vec2Util_moveTowards(velocity, targetVelocity, 500.0f * app_g.time.deltaTime);

					obj->rigidbody.velocity = velocity;
				}
			}

			// 時間経過で消滅.
			float aliveTime = app_g.time.time - obj->spawnedTime;
			if (0.5f <= aliveTime) {
				oskn_ObjList_requestRemoveById(&app_g.objList, id, 0.0f);
			}
			break;
		}
		}
	}

	// rigidbody.
	for (int i = 0, iCount = objList->activeIdListCount; i < iCount; ++i) {
		oskn_ObjId aId = objList->activeIdList[i];
		oskn_Obj* aObj = NULL;
		if (!oskn_ObjList_getById(objList, aId, &aObj)) continue;
		if (!aObj->rigidbody.enabled) continue;
		oskn_Vec2 pos = aObj->transform.position;
		oskn_Vec2 v = oskn_Vec2Util_mulF(aObj->rigidbody.velocity, app_g.time.deltaTime);
		pos = oskn_Vec2Util_addVec2(pos, v);
		aObj->transform.position = pos;
		aObj->rigidbody.hitT = 1.0f;
	}

	// 衝突判定.
	oskn_ObjHitInfoList_clear(&self->hitInfoList);
	for (int i = 0, iCount = objList->activeIdListCount; i < iCount; ++i) {
		oskn_ObjId aId = objList->activeIdList[i];
		oskn_Obj* aObj = NULL;
		if (!oskn_ObjList_getById(objList, aId, &aObj)) continue;
		oskn_Collider aCol = aObj->collider;

		for (int j = i + 1, jCount = objList->activeIdListCount; j < jCount; ++j) {
			oskn_ObjId bId = objList->activeIdList[j];
			oskn_Obj* bObj = NULL;
			if (!oskn_ObjList_getById(objList, bId, &bObj)) continue;
			if (!oskn_Obj_isNeedHitTest(aObj, bObj)) continue;

			oskn_Vec2 aPos = aObj->transform.position;
			oskn_Vec2 bPos = bObj->transform.position;
			oskn_Collider bCol = bObj->collider;

			float radius = aCol.radius + bCol.radius;
			float sqrRadius = radius * radius;
			oskn_Vec2 aToB = oskn_Vec2Util_subVec2(bPos, aPos);
			float sqrDistance = oskn_Vec2_sqrMagnitude(aToB);
			bool isHit = sqrDistance < sqrRadius;
			if (!isHit) continue;
			//oskn_Obj aObjTemp = *aObj;
			oskn_Vec2 hitPos = { 0 };
			bool isTrigger = aObj->rigidbody.isTrigger || bObj->rigidbody.isTrigger;
			oskn_Obj* prevAObj = NULL;
			if (!oskn_ObjList_tryGetById(&app_g.prevObjList, aObj->id, &prevAObj)) {
				prevAObj = aObj;
			}
			oskn_Obj* prevBObj = NULL;
			if (!oskn_ObjList_tryGetById(&app_g.prevObjList, bObj->id, &prevBObj)) {
				prevBObj = bObj;
			}
			oskn_HitInfo hitInfo = { 0 };
			if (!oskn_App_reflectObj(aObj, bObj, prevAObj->transform.position, prevBObj->transform.position, &app_g.time, &hitInfo)) {
				assert(false);
			}
			hitPos = hitInfo.hitPosition;
			if (!isTrigger) {
				if (OSKN_COL_POS_ADJUST_DELAY_ENABLED) {
					aObj->rigidbody.nextPosition = hitInfo.aPosition;
					aObj->rigidbody.nextVelocity = hitInfo.aVelocity;
					bObj->rigidbody.nextPosition = hitInfo.bPosition;
					bObj->rigidbody.nextVelocity = hitInfo.bVelocity;
				} else {
					aObj->transform.position = hitInfo.aPosition;
					aObj->rigidbody.velocity = hitInfo.aVelocity;
					bObj->transform.position = hitInfo.bPosition;
					bObj->rigidbody.velocity = hitInfo.bVelocity;
				}
			}

			oskn_ObjHitInfo h = { 0 };
			h.aId = aObj->id;
			h.bId = bObj->id;
			h.hitPosition = hitPos;

			if (OSKN_COL_POS_ADJUST_DELAY_ENABLED) {
				++aObj->rigidbody.hitCount;
				++bObj->rigidbody.hitCount;
				oskn_ObjHitInfoList_add(&self->hitInfoList, h);
			}
			else {
				oskn_App_onHitObj(self, aObj, bObj, h.hitPosition);
				oskn_App_onHitObj(self, bObj, aObj, h.hitPosition);
			}
		}
	}

	if (OSKN_COL_POS_ADJUST_DELAY_ENABLED) {
		// 位置補正、反射ベクトルの反映.
		for (int i = 0, iCount = objList->activeIdListCount; i < iCount; ++i) {
			oskn_ObjId aId = objList->activeIdList[i];
			oskn_Obj* aObj = NULL;
			if (!oskn_ObjList_getById(objList, aId, &aObj)) continue;
			if (aObj->rigidbody.hitCount <= 0) continue;
			aObj->transform.position = aObj->rigidbody.nextPosition;
			aObj->rigidbody.velocity = aObj->rigidbody.nextVelocity;
			aObj->rigidbody.hitCount = 0;
			aObj->rigidbody.hitT = 0.0f;
		}

		for (int i = 0, iCount = self->hitInfoList.count; i < iCount; ++i) {
			oskn_ObjHitInfo h = oskn_ObjHitInfoList_get(&self->hitInfoList, i);
			oskn_Obj* aObj = NULL;
			if (!oskn_ObjList_getById(objList, h.aId, &aObj)) continue;
			oskn_Obj* bObj = NULL;
			if (!oskn_ObjList_getById(objList, h.bId, &bObj)) continue;
			oskn_App_onHitObj(self, aObj, bObj, h.hitPosition);
			oskn_App_onHitObj(self, bObj, aObj, h.hitPosition);
		}
	}

	// areaRect 衝突判定.
	for (int i = 0, iCount = objList->activeIdListCount; i < iCount; ++i) {
		oskn_ObjId id = objList->activeIdList[i];
		oskn_Obj* obj = NULL;
		if (!oskn_ObjList_getById(objList, id, &obj)) continue;

		switch (obj->type) {
		case oskn_ObjType_Camera: {
			break;
		}
		case oskn_ObjType_DirectionMarker: {
			break;
		}
		case oskn_ObjType_Player: {
			break;
		}
		case oskn_ObjType_PlayerBullet: {
			break;
		}
		case oskn_ObjType_Enemy: {
			oskn_Vec2 pos = obj->transform.position;
			oskn_Vec2 vec = obj->rigidbody.velocity;
			oskn_Vec2 rectMin = oskn_Rect_min(app_g.areaRect);
			oskn_Vec2 rectMax = oskn_Rect_max(app_g.areaRect);

			if (pos.x - obj->collider.radius < rectMin.x) {
				vec = oskn_App_reflectVec(vec, oskn_Vec2_normalize(oskn_Vec2Util_create(1, 0)));
				pos.x = rectMin.x + obj->collider.radius;
			}
			else if (rectMax.x <= pos.x + obj->collider.radius) {
				vec = oskn_App_reflectVec(vec, oskn_Vec2_normalize(oskn_Vec2Util_create(-1, 0)));
				pos.x = rectMax.x - obj->collider.radius;
			}

			if (pos.y - obj->collider.radius < rectMin.y) {
				vec = oskn_App_reflectVec(vec, oskn_Vec2_normalize(oskn_Vec2Util_create(0, 1)));
				pos.y = rectMin.y + obj->collider.radius;
			}
			else if (rectMax.y <= pos.y + obj->collider.radius) {
				vec = oskn_App_reflectVec(vec, oskn_Vec2_normalize(oskn_Vec2Util_create(0, -1)));
				pos.y = rectMax.y - obj->collider.radius;
			}
			obj->transform.position = pos;
			obj->rigidbody.velocity = vec;
			break;
		}
		}
	}

	for (int i = objList->activeIdListCount - 1; 0 <= i; --i) {
		oskn_ObjId id = objList->activeIdList[i];
		oskn_Obj* obj = NULL;
		if (!oskn_ObjList_getById(objList, id, &obj)) continue;
		if (obj->destroyTime < app_g.time.time) continue;
		oskn_ObjList_removeById(objList, id);
	}

	oskn_ObjList_copyFrom(&self->prevObjList, &self->objList);
}

void oskn_App_createCamera(oskn_App* self) {
	oskn_Obj* camera = oskn_ObjList_add(&self->objList);
	camera->type = oskn_ObjType_Camera;
	self->cameraId = camera->id;
}

void oskn_App_createPlayer(oskn_App* self) {
	oskn_Vec2 areaCenter = oskn_Rect_center(self->areaRect);
	oskn_Obj* obj = oskn_ObjList_add(&app_g.objList);
	obj->type = oskn_ObjType_Player;
	obj->collider.radius = 12.0f;
	obj->player.hp = 1.0f;
	obj->transform.position = areaCenter;
	obj->player.shotInterval1 = 0.1f;
	obj->player.shotInterval2 = 0.05f;
	obj->player.shotFuelCapacity1 = 100;
	obj->player.shotFuelCapacity2 = 200;
	obj->player.shotFuelRest = obj->player.shotFuelCapacity1;
	obj->player.shotFuelRecover = obj->player.shotFuelCapacity1;
	obj->player.shotFuelConsume = obj->player.shotFuelCapacity1 / 3;

	oskn_Obj* marker = oskn_ObjList_add(&app_g.objList);
	marker->type = oskn_ObjType_DirectionMarker;
	marker->collider.radius = 8;
	marker->directionMarker.ownerId = obj->id;

	self->playerId = obj->id;
}

// 岩石の生成.
void oskn_App_updateEnemySpawn(oskn_App* self) {
	if (OSKN_COL_TEST_ENABLED) {
		if (0 < app_g.enemyAddCount) return;

		oskn_Obj* obj;
		obj = oskn_App_createEnemy(&app_g, oskn_Vec2Util_create(10, 50), 3);
		obj->rigidbody.velocity = oskn_Vec2Util_create(20, 0);

		obj = oskn_App_createEnemy(&app_g, oskn_Vec2Util_create(140, 50), 2);
		obj->rigidbody.velocity = oskn_Vec2Util_create(-40, 0);

		//obj = oskn_App_createEnemy(&app_g, oskn_Vec2Util_create(140, 140), 2);
		//obj->rigidbody.velocity = oskn_Vec2Util_create(0, -200);

		++app_g.enemyAddCount;

		return;
	}

	if (app_g.enemyAddCountMax <= app_g.enemyAddCount) return;
	float spawnInterval = 2.0f;
	INT32 prevSec = (INT32)((self->time.time - self->frameInterval) / spawnInterval);
	INT32 sec = (INT32)(self->time.time / spawnInterval);


	if (1.0f < self->time.time && (prevSec < sec)) {
		// spawn
		oskn_Vec2 pos = { 0 };
		int n = 4 * rand() / RAND_MAX;
		oskn_Vec2 rectMin = oskn_Rect_min(self->areaRect);
		oskn_Vec2 rectMax = oskn_Rect_max(self->areaRect);
		switch (n) {
		case 0:
		case 1:
			pos.x = (n == 0) ? rectMin.x : rectMax.x;
			pos.y = self->areaRect.y + self->areaRect.height * rand() / RAND_MAX;
			break;
		case 2:
		case 3:
		default:
			pos.x = self->areaRect.x + self->areaRect.width * rand() / RAND_MAX;
			pos.y = (n == 2) ? rectMin.y : rectMax.y;
			break;
		}

		oskn_App_createEnemy(&app_g, pos, 4);

		++app_g.enemyAddCount;
	}
}

void oskn_App_update(oskn_App* self) {
	oskn_Time_add(&self->time, self->frameInterval);
	oskn_Input_update(&self->input);

	oskn_AppState nextState = oskn_AppState_None;
	const int stateLoopLimit = 8;
	int stateLoopI = 0;
	do {
		bool isEnter = false;
		if (nextState != oskn_AppState_None) {
			self->appState = nextState;
			self->appStateStartTime = self->time.time;
			nextState = oskn_AppState_None;
			isEnter = true;
		}
		float stateTime = self->time.time - self->appStateStartTime;
		switch (self->appState) {
		case oskn_AppState_None: {
			nextState = oskn_AppState_Title;
			break;
		}
		case oskn_AppState_Title: {
			if (isEnter) {
				oskn_ObjList_clear(&self->objList);
				app_g.enemyAddCount = 0;
				app_g.enemyAddCountMax = 0;

				oskn_App_createCamera(self);
				oskn_App_createPlayer(self);
			}

			if (0.5f <= stateTime) {
				if (oskn_Input_hasKeyDown(&self->input, OSKN_KEY_START)) {
					nextState = oskn_AppState_Ready;
				}
			}
			break;
		}
		case oskn_AppState_Ready: {
			if (isEnter) {
				oskn_ObjList_clear(&self->objList);
				app_g.enemyAddCount = 0;
				app_g.enemyAddCountMax = 8;
				//app_g.enemyAddCountMax = 2;

				oskn_App_createCamera(self);
				oskn_App_createPlayer(self);
			}

			if (1.0f <= stateTime) {
				nextState = oskn_AppState_Main;
			}
			break;
		}
		case oskn_AppState_Main: {
			// ゲームクリア判定.
			{
				if (self->enemyAddCountMax <= self->enemyAddCount) {
					INT32 enemyCount = 0;
					for (int i = 0, iCount = self->objList.activeIdListCount; i < iCount; ++i) {
						oskn_ObjId objId = self->objList.activeIdList[i];
						oskn_Obj* obj = NULL;
						if (!oskn_ObjList_getById(&self->objList, objId, &obj)) continue;
						if (obj->type != oskn_ObjType_Enemy) continue;
						++enemyCount;
					}
					if (enemyCount <= 0) {
						nextState = oskn_AppState_Clear;
					}
				}
			}
			// ゲームオーバー判定.
			{
				oskn_Obj* player = NULL;
				if (!oskn_ObjList_getById(&self->objList, self->playerId, &player)) break;
				if (player->player.hp <= 0.0f) {
					nextState = oskn_AppState_Over;
				}
			}
			break;
		}
		case oskn_AppState_Over: {
			if (isEnter) {
				oskn_Obj* player = NULL;
				if (!oskn_ObjList_getById(&self->objList, self->playerId, &player)) break;
				oskn_ObjList_requestRemoveById(&app_g.objList, player->id, 0.0f);
			}
			if (0.5f <= stateTime) {
				if (oskn_Input_hasKeyDown(&self->input, OSKN_KEY_START)) {
					nextState = oskn_AppState_Title;
				}
			}
			if (3.0f <= stateTime) {
				nextState = oskn_AppState_Title;
			}
			break;
		}
		case oskn_AppState_Clear: {
			if (0.5f <= stateTime) {
				if (oskn_Input_hasKeyDown(&self->input, OSKN_KEY_START)) {
					nextState = oskn_AppState_Title;
				}
			}
			if (3.0f <= stateTime) {
				nextState = oskn_AppState_Title;
			}
			break;
		}
		}
		++stateLoopI;
	} while (nextState != oskn_AppState_None && stateLoopI < stateLoopLimit);

	oskn_App_updateEnemySpawn(&app_g);
	oskn_App_updateObj(self);
}

bool oskn_App_free(oskn_App* self) {
	oskn_ObjList_free(&self->objList);
	oskn_ObjHitInfoList_free(&self->hitInfoList);
	return true;
}


void draw(HWND hWnd) {
	TCHAR str[255];
	HDC hdc = app_g.hdcMem;

	RECT rcClient;
	GetClientRect(hWnd, &rcClient);

	SetBkColor(hdc, RGB(0xff, 0xff, 0xff));
	SetBkMode(hdc, OPAQUE);

	HFONT hFont = CreateFont(
		16, 0, 0, 0, FW_NORMAL, false, false, false,
		ANSI_CHARSET, OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		FIXED_PITCH | FF_MODERN, TEXT("ＭＳ ゴシック")
	);
	SelectObject(hdc, hFont);

	SelectObject(hdc, GetStockObject(BLACK_BRUSH));
	Rectangle(hdc, rcClient.left, rcClient.top, rcClient.right, rcClient.bottom);
	SelectObject(hdc, GetStockObject(WHITE_BRUSH));

	// カメラの数だけオブジェクトを描画する. 1 台しかないけど.
	for (int camI = 0, camICount = app_g.objList.activeIdListCount; camI < camICount; ++camI) {
		oskn_ObjId id = app_g.objList.activeIdList[camI];
		oskn_Obj* camera = NULL;
		if (!oskn_ObjList_getById(&app_g.objList, id, &camera)) continue;
		if (camera->type != oskn_ObjType_Camera) continue;
		oskn_Vec2 cameraOffset = oskn_Vec2Util_mulF(oskn_Vec2Util_addVec2(camera->transform.position, oskn_Vec2Util_mulF(app_g.screenSize, -0.5f)), -1.0f);

		{
			// areaRect の描画.
			HBRUSH hBrash = CreateSolidBrush(RGB(0xff, 0x00, 0xff));

			oskn_Vec2 rMin = oskn_Vec2Util_addVec2(oskn_Rect_min(app_g.areaRect), cameraOffset);
			oskn_Vec2 rMax = oskn_Vec2Util_addVec2(oskn_Rect_max(app_g.areaRect), cameraOffset);
			RECT r = { (int)rMin.x, (int)rMin.y, (int)rMax.x, (int)rMax.y };
			FrameRect(hdc, &r, hBrash);

			HBRUSH hBrashPrev = SelectObject(hdc, hBrash);
			SelectObject(hdc, hBrashPrev);
			DeleteObject(hBrash);
		}

		for (int i = 0, iCount = app_g.objList.activeIdListCount; i < iCount; ++i) {
			oskn_ObjId id = app_g.objList.activeIdList[i];
			oskn_Obj* obj = NULL;
			if (!oskn_ObjList_getById(&app_g.objList, id, &obj)) continue;
			POINT pt;
			pt.x = (int)(obj->transform.position.x + cameraOffset.x);
			pt.y = (int)(obj->transform.position.y + cameraOffset.y);
			RECT rc;
			COLORREF rgb;
			switch (obj->type) {
			case oskn_ObjType_Player:
				rgb = RGB(0x80, 0x00, 0xff);
				break;
			case oskn_ObjType_PlayerBullet:
				rgb = (obj->bullet.lv < 2) ?
					RGB(0x80, 0x00, 0xff) :
					RGB(0xc0, 0x80, 0xff);
				break;
			case oskn_ObjType_Fuel:
				rgb = RGB(0x80, 0x80, 0xff);
				break;
			case oskn_ObjType_Enemy:
			case oskn_ObjType_EnemyBullet:
				rgb = RGB(0xff, 0x00, 0x00);
				break;
			case oskn_ObjType_DirectionMarker:
				rgb = RGB(0xc0, 0xc0, 0xc0);
				break;
			default:
				rgb = RGB(0xff, 0x00, 0xff);
				break;
			}
			rc.left   = (int)(pt.x - obj->collider.radius);
			rc.top    = (int)(pt.y - obj->collider.radius);
			rc.right  = (int)(rc.left + (obj->collider.radius * 2.0f));
			rc.bottom = (int)(rc.top  + (obj->collider.radius * 2.0f));
			HBRUSH hBrash = CreateSolidBrush(rgb);
			HPEN hPen = CreatePen(PS_SOLID, 1, rgb);
			//FillRect(hdc, &rc, hBrash);
			HBRUSH hBrashPrev = SelectObject(hdc, hBrash);
			HPEN hPenPrev = SelectObject(hdc, hPen);
			Ellipse(hdc, rc.left, rc.top, rc.right, rc.bottom);
			//wsprintf(str, TEXT("%d"), obj->id.id);
			//TextOut(hdc, rc.left, rc.top, str, lstrlen(str));
			SelectObject(hdc, hPenPrev);
			SelectObject(hdc, hBrashPrev);
			DeleteObject(hPen);
			DeleteObject(hBrash);
		}
	}


	SIZE textSize;
	INT32 textX;
	INT32 textY;
	switch (app_g.appState) {
	case oskn_AppState_Title: {
		wsprintf(str, TEXT("タイトル"));
		GetTextExtentPoint32(hdc, str, lstrlen(str), &textSize);
		textX = (INT32)(app_g.screenSize.x * 0.5f - textSize.cx * 0.5f);
		textY = (INT32)(app_g.screenSize.y * 0.3f - textSize.cy * 0.5f);
		TextOut(hdc, textX, textY, str, lstrlen(str));

		wsprintf(str, TEXT("SPACE でスタート"));
		GetTextExtentPoint32(hdc, str, lstrlen(str), &textSize);
		textX = (INT32)(app_g.screenSize.x * 0.5f - textSize.cx * 0.5f);
		textY = (INT32)(app_g.screenSize.y * 0.5f + textSize.cy * 1.1f);
		TextOut(hdc, textX, textY, str, lstrlen(str));

		wsprintf(str, TEXT("ショット: Z"));
		GetTextExtentPoint32(hdc, str, lstrlen(str), &textSize);
		textX = (INT32)(app_g.screenSize.x * 0.5f - textSize.cx * 0.5f);
		textY = (INT32)(app_g.screenSize.y * 0.5f + textSize.cy * 2.2f);
		TextOut(hdc, textX, textY, str, lstrlen(str));

		wsprintf(str, TEXT("向き固定: SHIFT"));
		GetTextExtentPoint32(hdc, str, lstrlen(str), &textSize);
		textX = (INT32)(app_g.screenSize.x * 0.5f - textSize.cx * 0.5f);
		textY = (INT32)(app_g.screenSize.y * 0.5f + textSize.cy * 3.3f);
		TextOut(hdc, textX, textY, str, lstrlen(str));

		wsprintf(str, TEXT("移動: ↑↓←→"));
		GetTextExtentPoint32(hdc, str, lstrlen(str), &textSize);
		textX = (INT32)(app_g.screenSize.x * 0.5f - textSize.cx * 0.5f);
		textY = (INT32)(app_g.screenSize.y * 0.5f + textSize.cy * 4.4f);
		TextOut(hdc, textX, textY, str, lstrlen(str));
		break;
	}
	case oskn_AppState_Ready: {
		wsprintf(str, TEXT("READY"));
		GetTextExtentPoint32(hdc, str, lstrlen(str), &textSize);
		textX = (INT32)((app_g.screenSize.x - textSize.cx) * 0.5f);
		textY = (INT32)((app_g.screenSize.y - textSize.cy) * 0.5f);
		TextOut(hdc, textX, textY, str, lstrlen(str));
		break;
	}
	case oskn_AppState_Over: {
		wsprintf(str, TEXT("GAME OVER"));
		GetTextExtentPoint32(hdc, str, lstrlen(str), &textSize);
		textX = (INT32)((app_g.screenSize.x - textSize.cx) * 0.5f);
		textY = (INT32)((app_g.screenSize.y - textSize.cy) * 0.5f);
		TextOut(hdc, textX, textY, str, lstrlen(str));
		break;
	}
	case oskn_AppState_Clear: {
		wsprintf(str, TEXT("GAME CLEAR"));
		GetTextExtentPoint32(hdc, str, lstrlen(str), &textSize);
		textX = (INT32)((app_g.screenSize.x - textSize.cx) * 0.5f);
		textY = (INT32)((app_g.screenSize.y - textSize.cy) * 0.5f);
		TextOut(hdc, textX, textY, str, lstrlen(str));
		break;
	}
	}

	{
		oskn_Obj* player = NULL;
		if (oskn_ObjList_tryGetById(&app_g.objList, app_g.playerId, &player) && 0 < player->player.hp) {
			float fuelRest = player->player.shotFuelRest;
			// [        ]
			// [::::    ]
			// [::::::::]
			// [####::::]
			// [########]
			TCHAR gaugeStr[128] = { 0 };
			float cap1 = player->player.shotFuelCapacity1;
			float cap2 = player->player.shotFuelCapacity2;
			float barCount = 8;
			for (int i = 0; i < barCount; ++i) {
				float a = (i + 1) * cap1 / barCount;
				float b = cap1 + ((i + 1) * (cap2 - cap1) / barCount);
				PTCHAR c = (fuelRest < a) ?
					TEXT(" ") :
					(fuelRest < b) ?
						TEXT(":") :
						TEXT("#");
				lstrcat(gaugeStr, c);
			}
			wsprintf(str, TEXT("FUEL %3d [%s]"), (int)fuelRest, gaugeStr);

//			wsprintf(str, TEXT("FUEL %d"), (int)fuelRest);
			GetTextExtentPoint32(hdc, str, lstrlen(str), &textSize);
			textX = (INT32)(app_g.screenSize.x * 0.5f);
			textY = (INT32)(8.0f);

			TextOut(hdc, textX, textY, str, lstrlen(str));

		}

		wsprintf(str, TEXT("F %d T %d"), app_g.time.frameCount, (int)(app_g.time.time * 100));
		TextOut(hdc, 8, 10, str, lstrlen(str));
	}

	SelectObject(hdc, GetStockObject(SYSTEM_FONT));
	DeleteObject(hFont);
}

LRESULT CALLBACK myWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	HDC hdc;
	PAINTSTRUCT paint;

	switch (uMsg) {
	case WM_DESTROY:
		oskn_App_free(&app_g);
		PostQuitMessage(0);
		return 0;
	case WM_CREATE: {
		oskn_App_init(&app_g, hWnd);
		break;
	}
	case WM_PAINT: {
		RECT rcClient;
		GetClientRect(hWnd, &rcClient);
		hdc = BeginPaint(hWnd, &paint);

		draw(hWnd);

		StretchBlt(
			hdc, 0, 0, rcClient.right, rcClient.bottom,
			app_g.hdcMem, 0, 0, (int)app_g.screenSize.x, (int)app_g.screenSize.y, SRCCOPY);
		EndPaint(hWnd, &paint);
		return 0;
	}
	case WM_KEYDOWN: {
		oskn_Key key = oskn_Key_fromWParam(wParam);
		if (key != OSKN_KEY_NONE) {
			app_g.input.keyStateNext |= key;
		}
		return 0;
	}
	case WM_KEYUP: {
		oskn_Key key = oskn_Key_fromWParam(wParam);
		if (key != OSKN_KEY_NONE) {
			app_g.input.keyStateNext &= ~key;
		}
		return 0;
	}
	case WM_MOUSEMOVE: {
		//pt.x = LOWORD(lParam);
		//pt.y = HIWORD(lParam);
		//oskn_Obj* player = oskn_ObjList_get(&app_g.objList, app_g.playerId);
		//player->player.targetPosition.x = (float)pt.x;
		//player->player.targetPosition.y = (float)pt.y;
		break;
	}
	case WM_LBUTTONDOWN: {
		//pt.x = LOWORD(lParam);
		//pt.y = HIWORD(lParam);
		//oskn_Obj obj;
		//obj.type = oskn_ObjType_Enemy;
		//obj.transform.position.x = (float)pt.x;
		//obj.transform.position.y = (float)pt.y;

		//oskn_ObjList_add(&app_g.objList, &obj);
		return 0;
	}
	default:
		break;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

	PCTSTR appTitle = TEXT("岩石破壊");

	WNDCLASS wc;
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = myWndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = NULL;
	wc.hCursor = NULL;
	wc.hbrBackground = (HBRUSH)COLOR_BACKGROUND + 1;
	wc.lpszMenuName = NULL;
	wc.lpszClassName = TEXT("Osakana4242.Window");

	ATOM atom;
	if (!(atom = RegisterClass(&wc))) {
		MessageBox(NULL, TEXT("ウィンドウの登録に失敗"), NULL, MB_ICONERROR);
		return 0;
	}

	hInst_g = hInstance;

	HWND hWnd = CreateWindow(
		(LPCTSTR)atom, appTitle,
		WS_OVERLAPPEDWINDOW,
		//100, 100, 640, 480,
		CW_USEDEFAULT, CW_USEDEFAULT, 320 * 3, 240 * 3,
		NULL, NULL,
		hInstance, NULL);

	if (hWnd == NULL) {
		MessageBox(NULL, TEXT("ウィンドウの作成に失敗"), NULL, MB_ICONERROR);
		return 0;
	}

	ShowWindow(hWnd, nCmdShow);

	float totalTime = 0;
	UINT64 prevTime = GetTickCount64();
	while (true) {
		MSG msg;
		int result = PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE);
		if (result) {
			result = GetMessage(&msg, NULL, 0, 0);
			bool hasError = result == -1;
			if (hasError) {
				MessageBox(NULL, TEXT("メッセージの取得に失敗"), NULL, MB_ICONERROR);
				return 0;
			}
			bool isQuit = msg.message == WM_QUIT;
			if (isQuit) {
				return (int)msg.wParam;
			}
			DispatchMessage(&msg);
		}
		else {
			UINT64 time = GetTickCount64();
			float deltaTime = (INT32)(time - prevTime) * 0.001f;
			if (0.1f < deltaTime) {
				// 差分がありすぎるときはブレークポイントかけてるときとみなす.
				deltaTime = app_g.frameInterval;
			}
			totalTime += deltaTime;
			prevTime = time;
			if (totalTime < app_g.frameInterval) {

			}
			else {
				totalTime -= app_g.frameInterval;
				oskn_App_update(&app_g);
				InvalidateRect(hWnd, NULL, false);
			}

		}
	}
	return 0;
}

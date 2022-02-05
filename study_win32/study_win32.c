
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

typedef struct _oskn_Player {
	float hp;
	float shotInterval;
	float shotStartTime;
} oskn_Player;

typedef struct _oskn_Bullet {
	float damage;
	float speed;
} oskn_Bullet;

typedef struct _oskn_Enemy {
	float hp;
	float speed;
} oskn_Enemy;

typedef enum _oskn_ObjType {
	oskn_ObjType_None,
	oskn_ObjType_Player,
	oskn_ObjType_Enemy,
	oskn_ObjType_PlayerBullet,
	oskn_ObjType_EnemyBullet,
} oskn_ObjType;

typedef struct _oskn_Obj {
	bool destroyed;
	/// <summary>この時間に達していたら remove する. 0 以下は無効.</summary>
	float destroyTime;
	INT32 id;
	LPCWSTR name;
	float spawnedTime;

	oskn_ObjType type;
	oskn_Transform transform;
	oskn_Collider collider;
	oskn_Player player;
	oskn_Bullet bullet;
	oskn_Enemy enemy;

	void(*onHit)(struct _oskn_Obj* self, struct _oskn_Obj* other);
} oskn_Obj;

typedef struct _oskn_ObjList {
	oskn_Obj* totalList;
	int* activeList;
	int count;
	int capacity;
} oskn_ObjList;

typedef enum _oskn_Key {
	OSKN_KEY_NONE = 0,
	OSKN_KEY_LEFT  = 1 << 0,
	OSKN_KEY_RIGHT = 1 << 1,
	OSKN_KEY_UP    = 1 << 2,
	OSKN_KEY_DOWN  = 1 << 3,
	OSKN_KEY_SHOT  = 1 << 4,
	OSKN_KEY_FIX   = 1 << 5,
} oskn_Key;

typedef struct _oskn_Input {
	INT32 keyStatePrev;
	INT32 keyState;
	INT32 keyStateNext;
} oskn_Input;

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
	int playerId;
	float fps;
	float frameInterval;
	oskn_Vec2 screenSize;
	oskn_Rect areaRect;
	oskn_Time time;
	HBITMAP hBitmap;
	HDC     hdcMem;
	oskn_Input input;

	oskn_AppState appState;
	float appStateStartTime;
	
} oskn_App;

#define ANGLE_PI 3.141592f
#define DEG_TO_RAD (ANGLE_PI / 180.0f)
#define RAD_TO_DEG (180.0f / ANGLE_PI)

HINSTANCE hInst_g = NULL;
oskn_App app_g = { 0 };


bool oskn_Float_roundEq(float a, float b, float threshold) {
	float d = a - b;
	return -threshold <= d && d <= threshold;
}

float oskn_Angle_toRad(oskn_Angle* self) {
	return (*self) * DEG_TO_RAD;
}

float oskn_Angle_toDeg(oskn_Angle* self) {
	return *self;
}

float oskn_AngleUtil_fromRad(float rad) {
	return rad * RAD_TO_DEG;
}


void oskn_Time_add(oskn_Time* self, float deltaTime) {
	self->time += deltaTime;
	self->deltaTime = deltaTime;
	++self->frameCount;
}


void oskn_Vec2_init(oskn_Vec2* self, float x, float y) {
	self->x = x;
	self->y = y;
}

bool oskn_Vec2_isZero(const oskn_Vec2* self) {
	return self->x == 0.0f && self->y == 0.0f;
}

 float oskn_Vec2_sqrMagnitude(const oskn_Vec2* self) {
	return self->x * self->x + self->y * self->y;
}

float oskn_Vec2_magnitude(const oskn_Vec2* self) {
	return sqrtf(oskn_Vec2_sqrMagnitude(self));
}

void oskn_Vec2_normalize(oskn_Vec2* self) {
	if (self->x == 0 && self->y == 0) return;
	float mag = oskn_Vec2_magnitude(self);
	self->x /= mag;
	self->y /= mag;
}

oskn_Angle oskn_Vec2_toAngle(const oskn_Vec2* self) {
	if (self->x == 0 && self->y == 0) return 0.0f;
	float rad = atan2f(self->y, self->x);
	return oskn_AngleUtil_fromRad( rad );
}


oskn_Vec2 oskn_Vec2Util_fromAngle(oskn_Angle angle) {
	float rad = oskn_Angle_toRad(&angle);
	oskn_Vec2 vec;
	vec.x = cosf(rad);
	vec.y = sinf(rad);
	return vec;
}

bool oskn_Vec2Util_eq(const oskn_Vec2* a, const oskn_Vec2* b) {
	return a->x == b->x && a->y == b->y;
}

bool oskn_Vec2Util_roundEq(const oskn_Vec2* a, const oskn_Vec2* b, float threshold) {
	return
		oskn_Float_roundEq(a->x, b->x, threshold) &&
		oskn_Float_roundEq(a->y, b->y, threshold);
}


void oskn_Rect_init(oskn_Rect* self, float x, float y, float width, float height) {
	self->x = x;
	self->y = y;
	self->width = width;
	self->height = height;
}

oskn_Vec2 oskn_Rect_min(const oskn_Rect* self) {
	oskn_Vec2 vec;
	oskn_Vec2_init(&vec, self->x, self->y);
	return vec;
}

oskn_Vec2 oskn_Rect_max(const oskn_Rect* self) {
	oskn_Vec2 vec;
	oskn_Vec2_init(&vec, self->x + self->width, self->y + self->height);
	return vec;
}

oskn_Vec2 oskn_Rect_center(const oskn_Rect* self) {
	oskn_Vec2 vec;
	oskn_Vec2_init(&vec,
		self->x + self->width * 0.5f,
		self->y + self->height * 0.5f
	);
	return vec;
}


oskn_Key oskn_Key_fromWParam(WPARAM wParam) {
	switch (wParam) {
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
	oskn_Vec2 vec;
	oskn_Vec2_init(&vec, 0.0f, 0.0f);
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
		oskn_Vec2_normalize(&vec);
	}

	return vec;
}

void oskn_Input_update(oskn_Input* self) {
	self->keyStatePrev = self->keyState;
	self->keyState = self->keyStateNext;
}


bool oskn_Obj_isNeedHitTest(const oskn_Obj* self, const oskn_Obj* other) {
	oskn_ObjType type1 = min(self->type, other->type);
	oskn_ObjType type2 = max(self->type, other->type);

	switch (type1) {
	case oskn_ObjType_Player:
		switch (type2) {
		case oskn_ObjType_Enemy:
		case oskn_ObjType_EnemyBullet:
			return true;
		default: return false;
		}
	case oskn_ObjType_Enemy:
		switch (type2) {
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

void oskn_Obj_hit(oskn_Obj* self, oskn_Obj* other) {
	if (NULL == self->onHit) return;
	self->onHit(self, other);
}


bool oskn_ObjList_init(oskn_ObjList* self, int capacity) {
	self->capacity = capacity;
	self->totalList = calloc(self->capacity, sizeof(oskn_Obj));
	self->activeList = calloc(self->capacity, sizeof(int));
	for (int i = 1, iCount = self->capacity; i < iCount; ++i) {
		oskn_Obj* item = &self->totalList[i];
		item->destroyed = true;
	}

	return true;
}

bool oskn_ObjList_free(oskn_ObjList* self) {
	free(self->activeList);
	self->activeList = NULL;
	free(self->totalList);
	self->totalList = NULL;
	return true;
}

oskn_Obj* oskn_ObjList_get(oskn_ObjList* self, int id) {
	if (!id) return NULL;
	return &self->totalList[id];
}

int oskn_ObjList_add(oskn_ObjList* self, const oskn_Obj* obj) {
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

	*target = *obj;
	target->destroyed = false;
	target->destroyTime = 0.0f;
	target->id = i;
	target->spawnedTime = app_g.time.time;

	self->activeList[self->count] = i;
	++self->count;

	return i;
}

bool oskn_ObjList_requestRemove(oskn_ObjList* self, INT32 id, float time) {
	oskn_Obj* obj = oskn_ObjList_get(self, id);
	if (NULL == obj) return false;
	obj->destroyTime = app_g.time.time + time;
	return true;
}

bool oskn_ObjList_remove(oskn_ObjList* self, int id) {
	for (int i = self->count; 0 < i; --i) {
		int item = self->activeList[i];
		if (item != id) continue;

		for (int j = i, jCount = self->count; j < jCount; ++j) {
			self->activeList[j] = self->activeList[j + 1];
		}
		--self->count;

		oskn_Obj* obj = oskn_ObjList_get(self, id);
		obj->destroyed = true;
		return true;
	}
	return false;
}

void oskn_Player_onHit(oskn_Obj* aObj, oskn_Obj* bObj) {
	aObj->player.hp -= 1;
	if (aObj->player.hp <= 0) {
		oskn_ObjList_requestRemove(&app_g.objList, aObj->id, 0.0f);
	}
}

void oskn_PlayerBullet_onHit(oskn_Obj* aObj, oskn_Obj* bObj) {
	oskn_ObjList_requestRemove(&app_g.objList, aObj->id, 0.0f);
}

void oskn_Enemy_onHit(oskn_Obj* aObj, oskn_Obj* bObj) {
	if (bObj->type == oskn_ObjType_PlayerBullet) {
		aObj->enemy.hp -= bObj->bullet.damage;
		if (aObj->enemy.hp <= 0) {
			oskn_ObjList_requestRemove(&app_g.objList, aObj->id, 0.0f);
		}
	}
}

void oskn_App_test(oskn_App* self) {
	{
		oskn_Vec2 expected;
		oskn_Vec2 actual;

		oskn_Vec2_init(&expected, 1.0f, 0.0f);
		actual = oskn_Vec2Util_fromAngle(0);
		assert(oskn_Vec2Util_eq(&expected, &actual));

		oskn_Vec2_init(&expected, 0.0f, 1.0f);
		actual = oskn_Vec2Util_fromAngle(90);
		assert(oskn_Vec2Util_roundEq(&expected, &actual, 0.01f));

		oskn_Vec2_init(&expected, -1.0f, 0.0f);
		actual = oskn_Vec2Util_fromAngle(180);
		assert(oskn_Vec2Util_roundEq(&expected, &actual, 0.01f));

		oskn_Vec2_init(&expected, 0.0f, -1.0f);
		actual = oskn_Vec2Util_fromAngle(-90);
		assert(oskn_Vec2Util_roundEq(&expected, &actual, 0.01f));
	}
	{
		oskn_Vec2 vec;
		oskn_Angle expected;
		oskn_Angle actual;

		expected = 0.0f;
		oskn_Vec2_init(&vec, 1.0f, 0.0f);
		actual = oskn_Vec2_toAngle(&vec);
		assert(oskn_Float_roundEq(expected, actual, 0.01f));

		expected = 90.0f;
		oskn_Vec2_init(&vec, 0.0f, 1.0f);
		actual = oskn_Vec2_toAngle(&vec);
		assert(oskn_Float_roundEq(expected, actual, 0.01f));

		expected = 180.0f;
		oskn_Vec2_init(&vec, -1.0f, 0.0f);
		actual = oskn_Vec2_toAngle(&vec);
		assert(oskn_Float_roundEq(expected, actual, 0.01f));

		expected = -90.0f;
		oskn_Vec2_init(&vec, 0.0f, -1.0f);
		actual = oskn_Vec2_toAngle(&vec);
		assert(oskn_Float_roundEq(expected, actual, 0.01f));
	}
}

bool oskn_App_init(oskn_App* self, HWND hWnd) {
	oskn_App_test(self);

	oskn_ObjList_init(&self->objList, 255);
	self->screenSize.x = 320;
	self->screenSize.y = 240;
	self->fps = 60.0f;
	self->frameInterval = 1.0f / self->fps;
	self->areaRect.x = -8;
	self->areaRect.y = -8;
	self->areaRect.width = 320 + 16;
	self->areaRect.height = 240 + 16;
	HDC hdc;
	RECT rc;
	hdc = GetDC(hWnd);                      	// ウインドウのDCを取得
	GetClientRect(GetDesktopWindow(), &rc);  	// デスクトップのサイズを取得
	self->hBitmap = CreateCompatibleBitmap(hdc, rc.right, rc.bottom);
	self->hdcMem = CreateCompatibleDC(NULL);		// カレントスクリーン互換
	SelectObject(self->hdcMem, self->hBitmap);		// MDCにビットマップを割り付け

	self->appState = oskn_AppState_Title;
	return true;
}

void oskn_App_updateObj(oskn_App* self) {
	oskn_ObjList* objList = &self->objList;

	for (int i = 0, iCount = objList->count; i < iCount; ++i) {
		int id = objList->activeList[i];
		oskn_Obj* obj = oskn_ObjList_get(objList, id);

		switch (obj->type) {
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

				if (0.0f < obj->player.shotStartTime) {
					float time = app_g.time.time - obj->player.shotStartTime;
					float prevTime = time - app_g.time.deltaTime;
					INT32 count1 = (time == 0.0f) ? -1 : (INT32)(prevTime / obj->player.shotInterval);
					INT32 count2 = (INT32)(time / obj->player.shotInterval);
					if (count1 < count2) {
						oskn_Obj bullet = { 0 };
						bullet.type = oskn_ObjType_PlayerBullet;
						bullet.onHit = oskn_PlayerBullet_onHit;
						bullet.bullet.damage = 1.0f;
						bullet.bullet.speed = 400.0f;
						bullet.collider.radius = 8.0f;
						bullet.transform.position = obj->transform.position;
						bullet.transform.rotation = obj->transform.rotation;
						oskn_ObjList_add(&app_g.objList, &bullet);
					}

				}
			}


			if (!oskn_Vec2_isZero(&inputDir)) {
				oskn_Vec2 pos = obj->transform.position;
				oskn_Vec2 move;
				float speed = 100.0f * app_g.time.deltaTime;
				move.x = inputDir.x * speed;
				move.y = inputDir.y * speed;
				pos.x += move.x;
				pos.y += move.y;


				oskn_Vec2 rectMin = oskn_Rect_min(&app_g.areaRect);
				oskn_Vec2 rectMax = oskn_Rect_max(&app_g.areaRect);
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
					obj->transform.rotation = oskn_Vec2_toAngle(&move);
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
			oskn_Vec2 rectMin = oskn_Rect_min(&app_g.areaRect);
			oskn_Vec2 rectMax = oskn_Rect_max(&app_g.areaRect);

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
				oskn_ObjList_requestRemove(objList, id, 0.0f);
			}

			obj->transform.position = pos;
			obj->transform.rotation = oskn_Vec2_toAngle(&vec);
			break;
		}
		case oskn_ObjType_Enemy: {
			oskn_Vec2 vec = oskn_Vec2Util_fromAngle(obj->transform.rotation);
			float speed = obj->enemy.speed * app_g.time.deltaTime;
			vec.x *= speed;
			vec.y *= speed;
			oskn_Vec2 pos = obj->transform.position;
			pos.x += vec.x;
			pos.y += vec.y;
			oskn_Vec2 rectMin = oskn_Rect_min(&app_g.areaRect);
			oskn_Vec2 rectMax = oskn_Rect_max(&app_g.areaRect);

			if (pos.x - obj->collider.radius < rectMin.x) {
				vec.x *= -1;
				pos.x = rectMin.x + obj->collider.radius;
			}
			else if (rectMax.x <= pos.x + obj->collider.radius) {
				vec.x *= -1;
				pos.x = rectMax.x - obj->collider.radius;
			}

			if (pos.y - obj->collider.radius < rectMin.y) {
				vec.y *= -1;
				pos.y = rectMin.y + obj->collider.radius;
			}
			else if (rectMax.y <= pos.y + obj->collider.radius) {
				vec.y *= -1;
				pos.y = rectMax.y - obj->collider.radius;
			}
			obj->transform.position = pos;
			obj->transform.rotation = oskn_Vec2_toAngle(&vec);
			break;
		}
		}
	}

	// 衝突判定.
	for (int i = 0, iCount = objList->count; i < iCount; ++i) {
		int aId = objList->activeList[i];
		oskn_Obj* aObj = oskn_ObjList_get(objList, aId);
		oskn_Vec2 aPos = aObj->transform.position;
		oskn_Collider aCol = aObj->collider;

		for (int j = i + 1, jCount = objList->count; j < jCount; ++j) {
			int bId = objList->activeList[j];
			oskn_Obj* bObj = oskn_ObjList_get(objList, bId);
			if (!oskn_Obj_isNeedHitTest(aObj, bObj)) continue;

			oskn_Vec2 bPos = bObj->transform.position;
			oskn_Collider bCol = bObj->collider;

			float radius = aCol.radius + bCol.radius;
			float sqrRadius = radius * radius;
			oskn_Vec2 d;
			d.x = bPos.x - aPos.x;
			d.y = bPos.y - aPos.y;
			bool isHit = oskn_Vec2_sqrMagnitude(&d) < sqrRadius;
			if (!isHit) continue;
			oskn_Obj_hit(aObj, bObj);
			oskn_Obj_hit(bObj, aObj);
		}
	}

	for (int i = objList->count - 1; 0 <= i; --i) {
		int id = objList->activeList[i];
		oskn_Obj* obj = oskn_ObjList_get(objList, id);
		if (obj->destroyTime < app_g.time.time) continue;
		oskn_ObjList_remove(objList, id);
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
		case oskn_AppState_Title: {
			if (oskn_Input_hasKeyDown(&self->input, OSKN_KEY_SHOT)) {
				nextState = oskn_AppState_Ready;
			}
			break;
		}
		case oskn_AppState_Ready: {
			if (isEnter) {
				for (int i = 0, iCount = self->objList.count - 1; i < iCount; ++i) {
					int objId = self->objList.activeList[i];
					oskn_ObjList_requestRemove(&self->objList, objId, 0.0f);
				}

				oskn_Vec2 areaCenter = oskn_Rect_center(&self->areaRect);
				oskn_Obj obj = { 0 };
				obj.type = oskn_ObjType_Player;
				obj.collider.radius = 12.0f;
				obj.onHit = oskn_Player_onHit;
				obj.player.hp = 1.0f;
				oskn_Vec2_init(&obj.transform.position, areaCenter.x, areaCenter.y);
				obj.player.shotInterval = 0.1f;

				self->playerId = oskn_ObjList_add(&app_g.objList, &obj);
			}

			if (1.0f <= stateTime) {
				nextState = oskn_AppState_Main;
			}
			break;
		}
		case oskn_AppState_Main: {
			oskn_Obj* player = oskn_ObjList_get(&self->objList, self->playerId);
			if (player->player.hp <= 0.0f) {
				nextState = oskn_AppState_Over;
			}
			break;
		}
		case oskn_AppState_Over: {
			if (0.5f <= stateTime) {
				if (oskn_Input_hasKeyDown(&self->input, OSKN_KEY_SHOT)) {
					nextState = oskn_AppState_Title;
				}
			}
			if (3.0f <= stateTime) {
				nextState = oskn_AppState_Title;
			}
			break;
		}
		case oskn_AppState_Clear: {
			if (3.0f <= stateTime) {
				nextState = oskn_AppState_Title;
			}
			break;
		}
		}
		++stateLoopI;
	} while (nextState != oskn_AppState_None && stateLoopI < stateLoopLimit);

	// 岩石の生成.
	{
		float spawnInterval = 1.0f;
		INT32 prevSec = (INT32)((self->time.time - self->frameInterval) / spawnInterval);
		INT32 sec = (INT32)(self->time.time / spawnInterval);


		if (1.0f < self->time.time && (prevSec < sec)) {
			// spawn
			float x = self->areaRect.x + self->areaRect.width * rand() / RAND_MAX;
			float y = self->areaRect.y + self->areaRect.height * rand() / RAND_MAX;

			oskn_Obj obj = { 0 };
			obj.type = oskn_ObjType_Enemy;
			obj.transform.position.x = x;
			obj.transform.position.y = y;
			obj.collider.radius = 12.0f;
			obj.enemy.hp = 4;
			obj.onHit = oskn_Enemy_onHit;

			//		obj.transform.rotation = 360.0f * rand() / RAND_MAX;
			obj.transform.rotation = self->time.time * 10.0f;
			obj.enemy.speed = 25.0f + 75.0f * rand() / RAND_MAX;
			oskn_ObjList_add(&app_g.objList, &obj);

		}
	}

	oskn_App_updateObj(self);
}

bool oskn_App_free(oskn_App* self) {
	oskn_ObjList_free(&self->objList);
	return true;
}


void draw(HWND hWnd) {
	TCHAR str[255];
	HDC hdc = app_g.hdcMem;

	RECT rcClient;
	GetClientRect(hWnd, &rcClient);

	SetBkColor(hdc, RGB(0xff, 0xff, 0xff));
	SetBkMode(hdc, OPAQUE);

	SelectObject(hdc, GetStockObject(BLACK_BRUSH));
	Rectangle(hdc, rcClient.left, rcClient.top, rcClient.right, rcClient.bottom);
	SelectObject(hdc, GetStockObject(WHITE_BRUSH));
	for (int i = 0, iCount = app_g.objList.count; i < iCount; ++i) {
		int id = app_g.objList.activeList[i];
		oskn_Obj obj = *oskn_ObjList_get(&app_g.objList, id);
		POINT pt;
		pt.x = (int)obj.transform.position.x;
		pt.y = (int)obj.transform.position.y;
//		TextOut(hdc, pt.x, pt.y, lptStr, lstrlen(lptStr));
		RECT rc;
		COLORREF rgb;
		switch (obj.type) {
		case oskn_ObjType_Player:
		case oskn_ObjType_PlayerBullet:
			rgb = RGB(0x80, 0x00, 0xff);
			break;
		case oskn_ObjType_Enemy:
		case oskn_ObjType_EnemyBullet:
			rgb = RGB(0xff, 0x00, 0x00);
			break;
		default:
			rgb = RGB(0xff, 0x00, 0xff);
			break;
		}
		rc.left = (int)pt.x - 12;
		rc.top = (int)pt.y - 12;
		rc.right = rc.left + 24;
		rc.bottom = rc.top + 24;
		HBRUSH hBrash = CreateSolidBrush(rgb);
		FillRect(hdc, &rc, hBrash);
		DeleteObject(hBrash);
	}

	wsprintf(str, TEXT("F %d T %d"), app_g.time.frameCount, (int)(app_g.time.time * 100));
	TextOut(hdc, 10, 10, str, lstrlen(str));

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

		wsprintf(str, TEXT("ショット: Z"));
		GetTextExtentPoint32(hdc, str, lstrlen(str), &textSize);
		textX = (INT32)(app_g.screenSize.x * 0.5f - textSize.cx * 0.5f);
		textY = (INT32)(app_g.screenSize.y * 0.5f + textSize.cy * 1.1f);
		TextOut(hdc, textX, textY, str, lstrlen(str));

		wsprintf(str, TEXT("向き固定: SHIFT"));
		GetTextExtentPoint32(hdc, str, lstrlen(str), &textSize);
		textX = (INT32)(app_g.screenSize.x * 0.5f - textSize.cx * 0.5f);
		textY = (INT32)(app_g.screenSize.y * 0.5f + textSize.cy * 2.2f);
		TextOut(hdc, textX, textY, str, lstrlen(str));

		wsprintf(str, TEXT("移動: ↑↓←→"));
		GetTextExtentPoint32(hdc, str, lstrlen(str), &textSize);
		textX = (INT32)(app_g.screenSize.x * 0.5f - textSize.cx * 0.5f);
		textY = (INT32)(app_g.screenSize.y * 0.5f + textSize.cy * 3.3f);
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
				return msg.wParam;
			}
			DispatchMessage(&msg);
		}
		else {
			UINT64 time = GetTickCount64();
			float deltaTime = (INT32)(time - prevTime) * 0.001f;
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

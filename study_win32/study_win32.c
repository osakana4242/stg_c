
#include "math.h"
#include "assert.h"
#include "framework.h"

#define MAX_LOADSTRING 100

// グローバル変数:
HINSTANCE hInst;                                // 現在のインターフェイス
TCHAR szTitle[MAX_LOADSTRING];                  // タイトル バーのテキスト
TCHAR szWindowClass[MAX_LOADSTRING];            // メイン ウィンドウ クラス名

#define ANGLE_PI 3.141592f
typedef float oskn_Angle;

const float DEG_TO_RAD= ANGLE_PI / 180.0f;
const float RAD_TO_DEG = 180.0f / ANGLE_PI;

BOOL oskn_Float_roundEq(float a, float b, float threshold) {
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

typedef struct _oskn_Time {
	float time;
	int frameCount;
	float deltaTime;
} oskn_Time;

void oskn_Time_add( oskn_Time* self, float deltaTime) {
	self->time += deltaTime;
	self->deltaTime = deltaTime;
	++self->frameCount;
}

typedef struct _oskn_Vec2 {
	float x;
	float y;
} oskn_Vec2;

void oskn_Vec2_init(oskn_Vec2* self, float x, float y) {
	self->x = x;
	self->y = y;
}

BOOL oskn_Vec2_isZero(const oskn_Vec2* self) {
	return self->x == 0.0f && self->y == 0.0f;
}

float oskn_Vec2_magnitude(const oskn_Vec2* self) {
	float sqr = self->x * self->x + self->y * self->y;
	return sqrtf(sqr);
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

BOOL oskn_Vec2Util_eq(oskn_Vec2* a, oskn_Vec2* b) {
	return a->x == b->x && a->y == b->y;
}

BOOL oskn_Vec2Util_roundEq(const oskn_Vec2* a, const oskn_Vec2* b, float threshold) {
	return
		oskn_Float_roundEq(a->x, b->x, threshold) &&
		oskn_Float_roundEq(a->y, b->y, threshold);
}

typedef struct _oskn_Rect {
	float x;
	float y;
	float width;
	float height;
} oskn_Rect;

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

typedef struct _oskn_Transform {
	oskn_Vec2 position;
	oskn_Angle rotation;
} oskn_Transform;

typedef enum _oskn_ObjType {
	oskn_ObjType_None,
	oskn_ObjType_Player,
	oskn_ObjType_Enemy,
	oskn_ObjType_PlayerBullet,
	oskn_ObjType_EnemyBullet,
} oskn_ObjType;

typedef struct _oskn_Collider {
	float radius;
} oskn_Collider;

typedef struct _oskn_Player {
	oskn_Vec2 targetPosition;
} oskn_Player;

typedef struct _oskn_Enemy {
	INT32 hoge;
	float speed;
} oskn_Enemy;

typedef struct _oskn_Obj {
	BOOL destroyed;
	LPCWSTR name;
	float spawnedTime;
	oskn_ObjType type;
	oskn_Transform transform;
	oskn_Collider collider;
	oskn_Player player;
	oskn_Enemy enemy;
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

BOOL oskn_Input_hasKey(const oskn_Input* self, oskn_Key key) {
	return 0 != (self->keyState & key);
}

BOOL oskn_Input_hasKeyDown(const oskn_Input* self, oskn_Key key) {
	return
		(0 == (self->keyStatePrev & key)) &&
		(0 != (self->keyState & key));
}

BOOL oskn_Input_hasKeyUp(const oskn_Input* self, oskn_Key key) {
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
	
} oskn_App;

static oskn_App app_g;

BOOL oskn_ObjList_init(oskn_ObjList* self, int capacity) {
	self->capacity = capacity;
	self->totalList = calloc(self->capacity, sizeof(oskn_Obj));
	self->activeList = calloc(self->capacity, sizeof(int));
	for (int i = 1, iCount = self->capacity; i < iCount; ++i) {
		oskn_Obj* item = &self->totalList[i];
		item->destroyed = TRUE;
	}

	return TRUE;
}

BOOL oskn_ObjList_free(oskn_ObjList* self) {
	free(self->activeList);
	self->activeList = NULL;
	free(self->totalList);
	self->totalList = NULL;
	return TRUE;
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
	target->destroyed = FALSE;
	target->spawnedTime = app_g.time.time;

	self->activeList[self->count] = i;
	++self->count;

	return i;
}

BOOL oskn_ObjList_remove(oskn_ObjList* self, int id) {
	for (int i = self->count; 0 < i; --i) {
		int item = self->activeList[i];
		if (item != id) continue;

		for (int j = i, jCount = self->count; j < jCount; ++j) {
			self->activeList[j] = self->activeList[j + 1];
		}
		--self->count;

		oskn_Obj* obj = oskn_ObjList_get(self, id);
		obj->destroyed = TRUE;
		return TRUE;
	}
	return FALSE;
}

void oskn_ObjList_update(oskn_ObjList* self) {
	
	for (int i = 0, iCount = self->count; i < iCount; ++i) {
		int id = self->activeList[i];
		oskn_Obj* obj = oskn_ObjList_get(self, id);

		switch (obj->type) {
		case oskn_ObjType_Player: {
			oskn_Vec2 inputDir = oskn_Input_getDirection(&app_g.input);
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


				obj->transform.position = pos;
			}
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
			} else if (rectMax.x <= pos.x + obj->collider.radius) {
				vec.x *= -1;
				pos.x = rectMax.x - obj->collider.radius;
			}

			if (pos.y - obj->collider.radius < rectMin.y) {
				vec.y *= -1;
				pos.y = rectMin.y + obj->collider.radius;
			} else if (rectMax.y <= pos.y + obj->collider.radius) {
				vec.y *= -1;
				pos.y = rectMax.y - obj->collider.radius;
			}
			obj->transform.position = pos;
			obj->transform.rotation = oskn_Vec2_toAngle(&vec);
			break;
		}
		}
	}
}

BOOL oskn_App_init(oskn_App* self, HWND hWnd) {
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

	oskn_Vec2 areaCenter = oskn_Rect_center(&self->areaRect);
	oskn_Obj obj;
	obj.type = oskn_ObjType_Player;
	obj.collider.radius = 12.0f;
	obj.transform.position.x = areaCenter.x;
	obj.transform.position.y = areaCenter.y;
	self->playerId = oskn_ObjList_add(&app_g.objList, &obj);

	return TRUE;
}

void oskn_App_update(oskn_App* self) {
	oskn_Time_add(&self->time, self->frameInterval);
	oskn_Input_update(&self->input);

	// 岩石の生成.
	{
		float spawnInterval = 1.0f;
		INT32 prevSec = (INT32)((self->time.time - self->frameInterval) / spawnInterval);
		INT32 sec = (INT32)(self->time.time / spawnInterval);


		if (1.0f < self->time.time && (prevSec < sec)) {
			// spawn
			float x = self->areaRect.x + self->areaRect.width * rand() / RAND_MAX;
			float y = self->areaRect.y + self->areaRect.height * rand() / RAND_MAX;

			oskn_Obj obj;
			obj.type = oskn_ObjType_Enemy;
			obj.transform.position.x = x;
			obj.transform.position.y = y;
			obj.collider.radius = 12.0f;
			//		obj.transform.rotation = 360.0f * rand() / RAND_MAX;
			obj.transform.rotation = self->time.time * 10.0f;
			obj.enemy.speed = 25.0f + 75.0f * rand() / RAND_MAX;
			oskn_ObjList_add(&app_g.objList, &obj);

		}
	}

	oskn_ObjList_update(&self->objList);
}

BOOL oskn_App_free(oskn_App* self) {
	oskn_ObjList_free(&self->objList);
	return TRUE;
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
}

oskn_Key oskn_KeyUtil_fromWParam(WPARAM wParam) {
	switch (wParam) {
	case VK_LEFT:     return OSKN_KEY_LEFT;
	case 'A':         return OSKN_KEY_LEFT;
	case VK_RIGHT:    return OSKN_KEY_RIGHT;
	case 'D':         return OSKN_KEY_RIGHT;
	case VK_UP:       return OSKN_KEY_UP;
	case 'W':         return OSKN_KEY_UP;
	case VK_DOWN:     return OSKN_KEY_DOWN;
	case 'S':         return OSKN_KEY_DOWN;
	case VK_SHIFT:    return OSKN_KEY_FIX;
	case VK_RETURN:   return OSKN_KEY_SHOT;
	case VK_SPACE:    return OSKN_KEY_SHOT;
	default:          return OSKN_KEY_NONE;
	}
}

LRESULT CALLBACK myWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
#ifdef _DEBUG
	//TCHAR debugText[1024];
	//wsprintf(debugText, TEXT("uMsg=%d, wParam=%d, lParam=%d\n"), uMsg, wParam, lParam);
	//OutputDebugString(debugText);
#endif
	LPCTSTR lptStr = TEXT("ほげら");
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


		// BitBlt(hdc, 0, 0, rcClient.right, rcClient.bottom, app_g.hdcMem, 0, 0, SRCCOPY);
		StretchBlt(
			hdc, 0, 0, rcClient.right, rcClient.bottom,
			app_g.hdcMem, 0, 0, (int)app_g.screenSize.x, (int)app_g.screenSize.y, SRCCOPY);
		EndPaint(hWnd, &paint);
		return 0;
	}
	case WM_KEYDOWN: {
		oskn_Key key = oskn_KeyUtil_fromWParam(wParam);
		if (key != OSKN_KEY_NONE) {
			app_g.input.keyStateNext |= key;
		}
		return 0;
	}
	case WM_KEYUP: {
		oskn_Key key = oskn_KeyUtil_fromWParam(wParam);
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

	PCWSTR appTitle = L"うんこ";
	PCWSTR windowClass = L"STATIC";

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


    // TODO: ここにコードを挿入してください。

    // グローバル文字列を初期化する
	lstrcpy(szTitle, appTitle);
	lstrcpy(szWindowClass, windowClass);

//    MyRegisterClass(hInstance);

    // アプリケーション初期化の実行:
	hInst = hInstance; // グローバル変数にインスタンス ハンドルを格納する

	HWND hWnd = CreateWindow(
		(LPCTSTR)atom, szTitle,
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

	//CreateWindowEx(
	//	WS_EX_LEFT, TEXT("STATIC"), TEXT("Stand by Ready!!"),
	//	WS_CHILD | WS_VISIBLE,
	//	0, 0, 400, 100,
	//	hWnd, NULL, hInstance, NULL
	//);

	//CreateWindowEx(
	//	WS_EX_LEFT, TEXT("STATIC"), TEXT("Stand by Ready!!\nうんこ\nunko"),
	//	WS_CHILD | WS_VISIBLE,
	//	0, 100, 400, 100,
	//	hWnd, NULL, hInstance, NULL
	//);

	float totalTime = 0;
	UINT64 prevTime = GetTickCount64();
	while (TRUE) {
		MSG msg;
		int result = PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE);
		if (result) {
			result = GetMessage(&msg, NULL, 0, 0);
			BOOL hasError = result == -1;
			if (hasError) {
				MessageBox(NULL, TEXT("メッセージの取得に失敗"), NULL, MB_ICONERROR);
				return 0;
			}
			BOOL isQuit = msg.message == WM_QUIT;
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
				InvalidateRect(hWnd, NULL, FALSE);
			}

		}
	}
	return 0;
}

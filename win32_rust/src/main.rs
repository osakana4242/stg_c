
use std::{thread, time::{Duration, Instant}, rc::Rc, sync::Arc, ptr::NonNull};

use windows::{
	core::*,
	Win32::Foundation::*,
	Win32::Graphics::Gdi::ValidateRect,
	Win32::{
		System::LibraryLoader::GetModuleHandleA,
		Graphics::Gdi::*,
	},
	Win32::UI::WindowsAndMessaging::*,
};

#[derive(Default)]
struct App {
	time: i32,
	fps_actual: f32,
}

fn main() -> Result<()> {
	unsafe {
		let instance = GetModuleHandleA(None)?;
		debug_assert!(instance.0 != 0);

		let window_class = s!("window");
		let wc = WNDCLASSA {
			hCursor: LoadCursorW(None, IDC_ARROW)?,
			hInstance: instance,
			lpszClassName: window_class,

			style: CS_HREDRAW | CS_VREDRAW,
			lpfnWndProc: Some(wndproc),
			..Default::default()
		};

		let atom = RegisterClassA(&wc);
		debug_assert!(atom != 0);

		let hwnd = CreateWindowExA(
			WINDOW_EX_STYLE::default(),
			window_class,
			s!("This is a サンプル sample window"),
			WS_OVERLAPPEDWINDOW | WS_VISIBLE,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			None,
			None,
			instance,
			None,
		);

		let mut app = App { ..Default::default() };
		let hoge = NonNull::new_unchecked(&mut app as *mut App);
		SetWindowLongPtrA(hwnd, GWLP_USERDATA, hoge.as_ptr() as isize);
		let mut fuga: NonNull<App> = NonNull::new_unchecked(GetWindowLongPtrA(hwnd, GWLP_USERDATA) as *mut App);
		let ttt = fuga.as_mut().time;

		print!("start: {}", ttt);
		let mut message = MSG::default();

		let mut time = Instant::now();
		let mut fps_time = 0;
		let mut fps_fc = 0;
		let fps_interval = 3000;

		loop {
			let result: bool = PeekMessageA(&mut message, HWND(0), 0, 0, PM_REMOVE).into();
			if result {
				if message.message == WM_QUIT {
					return Ok(());
				}
				TranslateMessage(&message);
				DispatchMessageA(&message);
			} else {
				let time2 = Instant::now();
				let delta = time2 - time;
				if delta < Duration::from_millis(0) || delta < Duration::from_millis(16) {
					thread::sleep(Duration::from_millis(0));
				} else {
					time = time2;
					fps_fc += 1;
					fps_time += delta.as_millis() as u32;
					if fps_interval <= fps_time {
						print!("fps_time: {}\n", fps_time);
						app.fps_actual = fps_fc as f32 * 1000f32 / fps_time as f32;
						fps_time -= fps_interval;
						fps_fc = 0;
					}

					// update
					let mut fuga: NonNull<App> = NonNull::new_unchecked(GetWindowLongPtrA(hwnd, GWLP_USERDATA) as *mut App);
					let mut app = fuga.as_mut();
					app.time += 1;
					SetWindowLongPtrA(hwnd, GWLP_USERDATA, fuga.as_ptr() as isize);
					//
					InvalidateRect(hwnd, None, false);
				}
			}
		}

		// while GetMessageW(&mut message, HWND(0), 0, 0).into() {
		// 	DispatchMessageW(&message);
		// }

		// Ok(())
	}
}

// RGBマクロの定義が見つからないので用意する.
fn rgb(r: u8, g: u8, b: u8) -> COLORREF {
	return COLORREF(
		r as u32 |
		( g as u32 ) << 8 |
		( b as u32 ) << 16
	);
}

extern "system" fn wndproc(window: HWND, message: u32, wparam: WPARAM, lparam: LPARAM) -> LRESULT {
	unsafe {
		match message {
			WM_PAINT => {
				// println!("WM_PAINT");
				let mut lppaint: PAINTSTRUCT =  Default::default();
				let hdc = BeginPaint(window, &mut lppaint);
				let mut rc_client = RECT::default();
				GetClientRect(window, &mut rc_client);
				SetBkColor(hdc, rgb(0xff, 0xff, 0x00));
				SetBkMode(hdc, OPAQUE);
				DeleteObject(SelectObject(hdc, CreateSolidBrush(rgb(0xff, 0xff, 0xff))));
				//SelectObject(hdc, GetStockObject(BLACK_BRUSH));
				Rectangle(hdc, rc_client.left, rc_client.top, rc_client.right, rc_client.bottom);
			


				// ここの文字列の変換でクソハマる.
				//
				// fn encode(str: &str) -> Vec<u16> {
				// 	return str.encode_utf16().collect();
				// }
				// let text = encode("ふが");
				//
				// なら通るのに,
				//
				// let text = "ふが".encode_utf16().collect();
				//
				// だとなぜ通らないのか.
				// 型推論が出来ないからだった. 型指定すれば OK.
				//
				// let text = "ふが".encode_utf16().collect::<Vec<u16>>();

				let fuga: NonNull<App> = NonNull::new_unchecked(GetWindowLongPtrA(window, GWLP_USERDATA) as *mut App);
				let app = fuga.as_ref();
				let text = format!("ふが time: {}, fps: {:.1} hoge", app.time, app.fps_actual);
				let str = text.as_bytes();
				//let text3 = s!(str);
				let x = app.time % 200;
				SetBkMode(hdc, TRANSPARENT);
				TextOutA(hdc, x, 5, str);
				EndPaint(window, &mut lppaint);
				LRESULT(0)
			}
			WM_DESTROY => {
				println!("WM_DESTROY");
				PostQuitMessage(0);
				LRESULT(0)
			}
			_ => DefWindowProcA(window, message, wparam, lparam),
		}
	}
}

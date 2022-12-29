
use std::{thread, time::{Duration, Instant}};

use windows::{
	core::*,
	Win32::Foundation::*,
	Win32::Graphics::Gdi::ValidateRect,
	Win32::{
		System::LibraryLoader::GetModuleHandleA,
		Graphics::Gdi::{
			BeginPaint,
			EndPaint,
			PAINTSTRUCT,
			TextOutW, TextOutA, InvalidateRect
		}
	},
	Win32::UI::WindowsAndMessaging::*,
};

struct App {
	time: i32,
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

		let mut app = App { time: 0};
		let app_raw = app as *mut i32;

		SetWindowLongPtrA(hwnd, GWLP_USERDATA, (&app).to_owned());
		GetWindowLongPtrA(hwnd, GWLP_USERDATA);

		print!("start");
		let mut message = MSG::default();

		let time = Instant::now();

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
				if delta < Duration::from_millis(0) || Duration::from_millis(16) <= delta {
					thread::sleep(Duration::from_millis(1));
				} else {
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

extern "system" fn wndproc(window: HWND, message: u32, wparam: WPARAM, lparam: LPARAM) -> LRESULT {
	unsafe {
		match message {
			WM_PAINT => {
				println!("WM_PAINT");
				let mut lppaint: PAINTSTRUCT =  Default::default();
				let hdc = BeginPaint(window, &mut lppaint);

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
				let text3 = s!("ふが");
				TextOutA(hdc, 5, 5, text3.as_bytes());
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

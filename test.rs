#![no_std]

#[no_mangle]
pub fn RustAdd(left: i32, right: i32) -> i32 {
    left + right
}

#[repr(C)]
struct VECTOR {
    vx: i32,
    vy: i32,
    vz: i32,
    pad: i32,
}

struct SVECTOR {
    vx: i16,
    vy: i16,
    vz: i16,
    pad: i16,
}

extern "C" {
    fn SquareRoot12(a: i32) -> i32;
    fn VectorNormalS(v0: *mut VECTOR, v1: *mut SVECTOR) -> i32;
    fn printf(fmt: *const u8, ...) -> i32;
}

impl str {
    pub const fn as_ptr(&self) -> *const u8 {
        self as *const str as *const u8
    }
}

#[no_mangle]
pub fn RustFuz(i: u32) -> u32 {
    match (i % 3, i % 5) {
        (0, 0) => 1,
        (0, _) => 2,
        (_, 0) => 3,
        _ => 0,
    }
}

#[no_mangle]
pub fn Issoufle(i: i32) -> i32 {
    let v1 = VECTOR {
        vx: 45056,
        vy: 49152,
        vz: 53248,
        pad: 0,
    };
    let mut v2 = SVECTOR {
        vx: 0,
        vy: 0,
        vz: 0,
        pad: 0,
    };

    unsafe {
        VectorNormalS(&mut v1, &mut v2);

        printf(
            "rust: [%d %d %d]\n\0".as_ptr(),
            v2.vx as i32,
            v2.vy as i32,
            v2.vz as i32,
        );

        printf("SquareRoot 12 %d\n\0".as_ptr(), SquareRoot12(i));
    }

    1337
}

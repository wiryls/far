use std::{process::Command, path::PathBuf};

fn join(v: &[&'static str]) -> PathBuf {
    v.iter().collect::<PathBuf>()
}

fn main() {
    const DIR: &'static str = env!("CARGO_MANIFEST_DIR");
    let dir = [DIR, "res"];
    let src = [DIR, "res", "res.xml"];
    let dst = [DIR, "src", "fur", "res.gresource"];

    if let Err(e) = Command::new("glib-compile-resources")
        .arg("--target"   ).arg(join(&dst))
        .arg("--sourcedir").arg(join(&dir))
        .arg(join(&src))
        .status()
    {
        use std::io::ErrorKind::NotFound;
        match e.kind() {
            NotFound => println!("missing {}", "glib-compile-resources"),
            _        => println!("whooops {}", e),
        }
    }

    println!("cargo:rerun-if-changed=res/res.xml");
}

// References:
//
// [The Cargo Book: Build Scripts]
// (https://doc.rust-lang.org/cargo/reference/build-scripts.html)
//
// [Getting Started With Rust and GTK]
// (https://blog.sb1.io/getting-started-with-rust-and-gtk/)
//
// [Support cargo:info=MESSAGE in build.rs stdout]
// (https://github.com/rust-lang/cargo/issues/7037)
//
// [glib-compile-resources]
// (https://developer.gnome.org/gio/stable/glib-compile-resources.html)

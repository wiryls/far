use far::Diff;
use gtk::{glib, gio};
use gtk::subclass::prelude::*;

#[derive(Default)]
pub struct Item {
    stat: u32,
    path: Path,
    diff: Diff,
}

#[glib::object_subclass]
impl ObjectSubclass for Item {
    const NAME: &'static str = "com.github.wiryls.far.fur.item";
    type Type = super::Item;
    type ParentType = glib::Object;
}

impl ObjectImpl for Item {

}

pub const StateImported : u32 = 0b00000001;
pub const StateDiffered : u32 = 0b00000010;
pub const StateReplaced : u32 = 0b00000100;

#[derive(Default)]
pub struct Path(std::path::PathBuf);

impl<'a> Path {

    pub fn set(&mut self, path : &str) {
        self.0 = std::path::PathBuf::from(path)
    }

    pub fn get(&'a self) -> &'a str {
        self.0
            .to_str()
            .unwrap_or(&"")
    }

    pub fn base(&'a self) -> &'a str {
        self.0
            .file_name()
            .map_or(None, std::ffi::OsStr::to_str)
            .unwrap_or(&"")
    }

    pub fn dir(&'a self) -> &'a str {
        self.0
            .parent()
            .map_or(None, std::path::Path::to_str)
            .unwrap_or(&"")
    }
}

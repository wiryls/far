use std::path;
use far::Diff;
use gtk::{glib, gio};
use gtk::subclass::prelude::*;

#[derive(Default)]
struct FilePath(path::PathBuf);

impl<'a> FilePath {

    fn set(&mut self, filepath : &str) {
        self.0 = path::PathBuf::from(filepath)
    }

    fn path(&'a self) -> &'a str {
        self.0
            .to_str()
            .unwrap_or(&"")
    }

    fn base(&'a self) -> &'a str {
        self.0
            .file_name()
            .map(|x| x.to_str())
            .unwrap_or(None)
            .unwrap_or(&"")
    }

    fn dir(&'a self) -> &'a str {
        self.0
            .parent()
            .map(|x| x.to_str())
            .unwrap_or(None)
            .unwrap_or(&"")
    }
}

#[derive(Default)]
pub struct Item {
    name: FilePath,
    diff: Diff,
}

#[glib::object_subclass]
impl ObjectSubclass for Item {
    const NAME: &'static str = "com.github.wiryls.far.fur.item";
    type Type = super::Item;
    type ParentType = glib::Object;

}

impl ObjectImpl for Item {}

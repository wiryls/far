//! A module for our GUI

mod list;
use gtk::{glib, gio};

glib::wrapper! {
    pub struct List(ObjectSubclass<list::List>) @implements gio::ListModel;
}

impl Default for List {
    fn default() -> Self {
        Self::new()
    }
}

impl List {
    pub fn new() -> Self {
        glib::Object::new(&[]).expect("Failed to create List")
    }
}

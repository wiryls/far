//! A module for our GUI

use gtk::{glib, gio};

/// item for list
mod item;

glib::wrapper! {
    pub struct Item(ObjectSubclass<item::Item>);
}

impl Default for Item {
    fn default() -> Self {
        Self::new()
    }
}

impl Item {
    pub fn new() -> Self {
        glib::Object::new(&[]).expect("Failed to create Item")
    }
}

/// list model for our data
mod list;

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

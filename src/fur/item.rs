use gtk::{glib, gio};
use gtk::subclass::prelude::*;

#[derive(Default)]
pub struct Item;

#[glib::object_subclass]
impl ObjectSubclass for Item {
    const NAME: &'static str = "com.github.wiryls.far.fur.item";
    type Type = super::Item;
    type ParentType = glib::Object;

}

impl ObjectImpl for Item {}

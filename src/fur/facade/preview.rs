//! The main window for finding, replacing and previewing.

// References:
//
// [GUI development with Rust and GTK 4: Subclassing]
// (https://gtk-rs.org/gtk4-rs/gi, width: (), height: () t/book/gobject_subclassing.html)
//
// [gtk4-rs examples composite_template]
// (https://github.com/gtk-rs/gtk4-rs/tree/master/examples)
//
// [gtk4 getting started]
// (https://docs.gtk.org/gtk4/getting_started.html#a-menu)
//
// [Gtk4-tutorial: GtkMenuButton, accelerators, font, pango and gsettings]
// (https://github.com/ToshioCP/Gtk4-tutorial/blob/main/gfm/sec20.md)
//
// [Using GMenu]
// (https://developer.gnome.org/GMenu/)

use gtk::{glib, gdk, gio};
use gtk::{prelude::*, subclass::prelude::*};
use gtk::CompositeTemplate;
use glib::clone;
use crate::fur::Context;

#[derive(Debug, Default, CompositeTemplate)]
#[template(resource = "/com/github/wiryls/far/ui/preview.ui")]
pub struct Window {

    #[template_child]
    pub input_pattern: TemplateChild<gtk::Entry>,

    #[template_child]
    pub input_replace: TemplateChild<gtk::Entry>,

    #[template_child]
    pub button_rename: TemplateChild<gtk::Button>,

    #[template_child]
    pub table: TemplateChild<gtk::ColumnView>,

    #[template_child]
    pub table_menu: TemplateChild<gtk::PopoverMenu>,

    #[template_child]
    pub column_stat: TemplateChild<gtk::ColumnViewColumn>,

    #[template_child]
    pub column_view: TemplateChild<gtk::ColumnViewColumn>,

    #[template_child]
    pub column_path: TemplateChild<gtk::ColumnViewColumn>,
}

impl Window {

    // setup will be triggered after "constructed"
    //
    // note: the sequence of funcion call is:
    //    1. ObjectSubclass::class_init
    //    2. ObjectSubclass::instance_init
    //    3. ObjectImpl::constructed
    //    4. setup
    //    5. ObjectImpl::dispose
    pub fn setup(&self, ctx: &Context) {

        let right_click = gtk::GestureClick::builder()
            .button(gtk::gdk::BUTTON_SECONDARY)
            .build();

        let table_menu = &*self.table_menu;
        right_click.connect_released(clone!(@weak table_menu => move |gesture, _, x, y|{
            // consume the message
            gesture.set_state(gtk::EventSequenceState::Claimed);

            let rect = gdk::Rectangle{x: x as i32,y: y as i32, width: 0, height: 0};
            table_menu.set_pointing_to(&rect);
            table_menu.popup();
        }));

        self.table.add_controller(&right_click);

        // References: Gesture
        //
        // [Gesture]
        // (https://gnome.pages.gitlab.gnome.org/gtk/gtk4/class.Gesture.html)

        self.setup_actions(ctx);
    }

    fn setup_actions(&self, _ctx: &Context) {
        let this = self.instance();

        {   // action group "win"
            {
                let action = gio::SimpleAction::new("import", None);
                this.add_action(&action);

                action.connect_activate(glib::clone!(@weak this => move |_, _| {
                    let chooser = gtk::FileChooserNative::new(
                        Some("Import"),
                        Some(&this),
                        gtk::FileChooserAction::Open,
                        Some("_OK"),
                        Some("_Cancel"));

                    chooser.set_select_multiple(true);
                    chooser.connect_accept_label_notify(|chooser: &gtk::FileChooserNative| {
                        println!("import files");
                        chooser.destroy();
                    });
                    chooser.show();
                }));
            }
        }

        {   // action group "settings"
            let group = gio::SimpleActionGroup::new();
            this.insert_action_group("settings", Some(&group));
            {
                let action = gio::SimpleAction::new_stateful(
                    "case-sensitive",
                    None,
                    &true.to_variant());
                group.add_action(&action);

                action.connect_activate(|action, _| {
                    if let Some(value) = action.state().and_then(|x| x.get::<bool>()) {
                        action.set_state(&(!value).to_variant());
                        println!("case-sensitive {}", !value);
                    }
                });
            }
            {
                let action = gio::SimpleAction::new_stateful(
                    "use-regexp",
                    None,
                    &true.to_variant());
                group.add_action(&action);

                action.connect_activate(|action, _| {
                    if let Some(value) = action.state().and_then(|x| x.get::<bool>()) {
                        action.set_state(&(!value).to_variant());
                        println!("use-regexp {}", !value);
                    }
                });
            }
        }

        {   // action group "table"
            let group = gio::SimpleActionGroup::new();
            self.table.insert_action_group("table", Some(&group));
            {
                let action = gio::SimpleAction::new("delete", None);
                group.add_action(&action);

                // enable if selection is not empty.
                action.connect_activate(|_, _| {
                    println!("delete some items");
                });
            }
            {
                let action = gio::SimpleAction::new("clear", None);
                group.add_action(&action);

                action.connect_activate(|_, _| {
                    println!("clear all items");
                });
            }
        }

        // References: Action
        //
        // [Custom widgets in GTK 4 – Actions]
        // (https://blog.gtk.org/2020/05/01/custom-widgets-in-gtk-4-actions/)
        //
        // [GAction]
        // (https://developer.gnome.org/GAction/)
        //
        // [Action groups and action maps]
        // (https://developer.gnome.org/gtk4/stable/ch06s04.html)
        //
        // [gtkmm4 - Actions]
        // (https://developer.gnome.org/gtkmm-tutorial/stable/sec-actions.html.en)
        //
        // [Gio.Menu items always disabled for boolean actions]
        // (https://stackoverflow.com/questions/66509574)
        //
        // [examples custom_widget button]
        // (https://github.com/gtk-rs/gtk4-rs/blob/af3a136457f3623bf6c4b3cf94a6a1b8652415bf/examples/custom_widget/ex_button/imp.rs)
        //
        // [ToshioCP: Stateful action]
        // (https://github.com/ToshioCP/Gtk4-tutorial/blob/main/gfm/sec18.md)
    }
}

#[glib::object_subclass]
impl ObjectSubclass for Window {
    // should be same as what in *.ui file
    const NAME: &'static str = "PreviewWindow";
    // point to the wrappered type
    type Type = super::PreviewWindow;
    // this windows is derived from gtk::ApplicationWindow
    type ParentType = gtk::ApplicationWindow;

    fn class_init(klass: &mut Self::Class) {
        Self::bind_template(klass);

        // bind actions from the ui file to our window
        klass.install_action("win.quit", None, move |this, _action_name, _action_target| {
            let this = Window::from_instance(this);
            this.instance().close();
        });

        // References:
        //
        // [install action]
        // (https://github.com/gtk-rs/gtk4-rs/blob/6c296e5763faf8e3e967c457b318b8a8fbd45303/examples/video_player/video_player_window/imp.rs#L47)
    }

    fn instance_init(o: &glib::subclass::InitializingObject<Self>) {
        o.init_template();
    }
}

impl ObjectImpl for Window {

    fn constructed(&self, obj: &Self::Type) {
        self.parent_constructed(obj);
        self.table_menu.set_parent(&*self.table);
    }

    fn dispose(&self, _obj: &Self::Type) {
        if let Some(_) = self.table_menu.parent() {
            self.table_menu.unparent();
        }
    }

    // Note: we need to manage lifetime of our popovermenu manually or GTK
    // will pop up warning.
    // https://github.com/gtk-rs/gtk4-rs/blob/af3a136457f3623bf6c4b3cf94a6a1b8652415bf/examples/custom_widget/ex_button/imp.rs
    // https://github.com/gtk-rs/gtk4-rs/blob/af3a136457f3623bf6c4b3cf94a6a1b8652415bf/examples/composite_template/ex_menu_button/imp.rs
}

impl WidgetImpl for Window {}
impl WindowImpl for Window {}
impl ApplicationWindowImpl for Window {}

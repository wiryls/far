// note for naming conventions
// https://doc.rust-lang.org/1.0.0/style/style/naming/README.html

pub enum State {
    Idle,
    Loading,
    Preview,
    Diffing,
    Renaming,
}

pub enum Input {
    // Import files or directories
    Import(Vec<String>),
    // Update pattern and template
    Update(String, String),
    // Append items list.
    Append(/* TODO */),
    // Perform to rename 
    Rename(/* TODO */),
}

pub enum Output {
    /* TODO */
}

pub struct StateMachine {
    /* TODO */
}

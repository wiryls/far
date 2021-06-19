#[derive(Debug, Clone)]
pub enum Diff {
    Retain(String),
	Insert(String),
	Delete(String),
}

pub type Diffs = Vec<Diff>;

pub trait Compare {
    fn concat(&self, f: fn(&Diff) -> &str) -> String;
    fn new(&self) -> String;
    fn old(&self) -> String;
    fn unchanged(&self) -> bool;
}

impl Compare for Diffs {

    fn concat(&self, f: fn(&Diff) -> &str) -> String {
        self.into_iter()
            .map(f)
            .collect::<Vec<&str>>()
            .concat()
    }

    fn new(&self) -> String {
        self.concat(|s : &Diff| match s{
            Diff::Retain(s) => s.as_str(),
            Diff::Insert(s) => s.as_str(),
            Diff::Delete(_) => ""})
    }

    fn old(&self) -> String {
        self.concat(|s: &Diff| match s {
            Diff::Retain(s) => s.as_str(),
            Diff::Insert(_) => "",
            Diff::Delete(s) => s.as_str()})
    }

    fn unchanged(&self) -> bool {
        let v = &self;
        let l = v.len();
        (l == 0) || (l == 1 && matches!(v[0], Diff::Retain{..}))
    }
}

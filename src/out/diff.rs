pub trait Difference
{
    fn new(&self) -> String;
    fn old(&self) -> String;
    fn unchanged(&self) -> bool;
}

#[derive(Debug)]
pub enum Diff
{
    Retain(String),
	Insert(String),
	Delete(String),
}

pub type Diffs = Vec<Diff>;

impl Difference for Diffs
{
    fn new(&self) -> String
    {
        (&self)
            .into_iter()
            .map(|s : &Diff| match s {
                Diff::Retain(s) => s.as_str(),
                Diff::Insert(s) => s.as_str(),
                Diff::Delete(_) => ""})
            .collect::<Vec<&str>>()
            .concat()
    }

    fn old(&self) -> String
    {
        (&self)
            .into_iter()
            .map(|s: &Diff| match s {
                Diff::Retain(s) => s.as_str(),
                Diff::Insert(_) => "",
                Diff::Delete(s) => s.as_str()})
            .collect::<Vec<&str>>()
            .concat()
    }

    fn unchanged(&self) -> bool
    {
        let v = &self;
        let l = v.len();
        (l == 0) || (l == 1 && matches!(v[0], Diff::Retain{..}))
    }
}

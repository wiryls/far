// For now there is no small string optimization in Rust. We decide to keep
// some ranges instead of new strings.

/// Change is an operation to Retain, Delete or Insert a string.
/// It references to source string and a ChangeBuf.
#[derive(Clone, Debug)]
pub enum Change<'a> {
    Retain(&'a str),
    Delete(&'a str),
    Insert(&'a str),
}

/// ChangeBuf is an internal type.
#[derive(Clone, Debug)]
pub(in crate::far) enum ChangeBuf {
    Retain(std::ops::Range<usize>),
	Delete(std::ops::Range<usize>),
	Insert(String),
}

/// Diff is something releated to transform a source string to its target
/// string.
#[derive(Default)]
pub struct Diff(Vec<ChangeBuf>);

impl Diff {

    pub(in crate::far) fn from_vec(src: Vec<ChangeBuf>) -> Self {

        let mut dst = Vec::with_capacity(src.len());
        for rhs in src.into_iter() {
            use ChangeBuf::{Retain, Delete, Insert};
            match dst.last_mut() {
                Some(lhs) => match (lhs, &rhs) {
                    (Retain(l), Retain(r)) => l.end = r.end,
                    (Delete(l), Delete(r)) => l.end = r.end,
                    (Insert(l), Insert(r)) => l.push_str(r.as_str()),
                    _ => dst.push(rhs),
                },
                None => dst.push(rhs),
            };
        }

        dst.shrink_to_fit();
        Self(dst)
    }

    pub fn iter<'a>(&'a self, source : &'a str) -> DiffIterator<'a> {
        DiffIterator{
            index: 0,
            mapper: self,
            source,
        }
    }

    pub fn is_same(&self) -> bool {
        return self.0.is_empty();
    }

    pub fn target(&self, source: &str) -> String {
        self.iter(source)
            .filter_map(|x| match x {
                Change::Retain(s) => Some(s),
                Change::Delete(_) => None,
                Change::Insert(s) => Some(s)})
            .collect()
    }
}

pub struct DiffIterator<'a> {
    index: usize,
    mapper: &'a Diff,
    source: &'a str,
}

impl<'a> Iterator for DiffIterator<'a> {

    type Item = Change<'a>;

    fn next(&mut self) -> Option<Self::Item> {
        match self.mapper.0.get(self.index) {
            Some(d) => {
                self.index += 1;
                Some(match d {
                    ChangeBuf::Retain(r) => Change::Retain(&self.source[r.clone()]),
                    ChangeBuf::Delete(r) => Change::Delete(&self.source[r.clone()]),
                    ChangeBuf::Insert(s) => Change::Insert(s),
                })
            },
            None => match self.index {
                0 => {
                    self.index += 1;
                    Some(Change::Retain(&self.source))
                },
                _ => None,
            }
        }
    }
}

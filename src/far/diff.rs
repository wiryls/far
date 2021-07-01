// Currently there is no small string optimization in Rust. We decide to keep
// pairs of integers to record ranges instead of store new strings.

#[derive(Clone, Debug)]
pub enum Change<'a> {
    Retain(&'a str),
    Delete(&'a str),
    Insert(&'a str),
}

#[derive(Clone, Debug)]
pub(in crate::far) enum ChangeBuf {
    Retain(std::ops::Range<usize>),
	Delete(std::ops::Range<usize>),
	Insert(String),
}

#[derive(Default)]
pub struct Diff {
    v: Vec<ChangeBuf>
}

impl Diff {

    pub(in crate::far) fn new(v: Vec<ChangeBuf>) -> Self {
        Self {v}
    }

    pub fn iter<'a>(&'a self, source : &'a str) -> DiffIterator<'a> {
        DiffIterator{
            index: 0,
            values: self,
            source,
        }
    }

    pub fn is_same(&self) -> bool {
        let v = &self.v;
        let l = self.v.len();
        (l == 0) || (l == 1 && matches!(v[0], ChangeBuf::Retain{..}))
    }

    pub fn target(&self, source: &str) -> String {
        // self.iter(source)
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
    values: &'a Diff,
    source: &'a str,
}

impl<'a> Iterator for DiffIterator<'a> {

    type Item = Change<'a>;

    fn next(&mut self) -> Option<Self::Item> {
        match self.values.v.get(self.index) {
            None => None,
            Some(d) => {
                self.index += 1;
                Some(match d {
                    ChangeBuf::Retain(r) => Change::Retain(&self.source[r.clone()]),
                    ChangeBuf::Delete(r) => Change::Delete(&self.source[r.clone()]),
                    ChangeBuf::Insert(s) => Change::Insert(s),
                })
            },
        }
    }
}

use std::fmt;
use super::{Diff, Faregex};

#[derive(Debug, Clone)]
pub enum Error {
    InvalidPattern (String),
    InvalidTemplate(String),
}

impl fmt::Display for Error {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        match &self {
            Error::InvalidPattern (s) => write!(f, "invalid pattern: {}", &s),
            Error::InvalidTemplate(s) => write!(f, "invalid template: {}", &s),
        }
    }
}

pub trait Far {
    fn see         (&self, text : &str) -> Diff<String>;
    fn is_empty    (&self)              -> bool;

    // could someone tell me whether it is ok to use
    // Option<Error> instead of Result<(), Error>？
    fn set_pattern (&mut self, pattern  : &str) -> Option<Error>;
    fn get_pattern (&self)              -> &str;
    fn set_template(&mut self, template : &str) -> Option<Error>;
    fn get_template(&self)              -> &str;
}

impl Far for Faregex {

    fn see(&self, text : &str) -> Diff<String> {
        self.see(text)
    }

    fn is_empty(&self) -> bool {
        self.is_empty()
    }

    fn set_pattern(&mut self, pattern : &str) -> Option<Error> {
        use regex::Error as E;
        use Error::InvalidPattern;
        self.set_pattern(pattern).and_then(|e| Some(match e {
            E::CompiledTooBig(_) => InvalidPattern("compiled regex is too long".to_owned()),
            E::Syntax        (s) => InvalidPattern(s),
            _                    => InvalidPattern("unknown error".to_owned()),
        }))
    }

    fn get_pattern(&self) -> &str {
        self.get_pattern()
    }

    fn set_template(&mut self, template : &str) -> Option<Error> {
        self.set_template(template);
        None
    }

    fn get_template(&self) -> &str {
        self.get_template()
    }
}

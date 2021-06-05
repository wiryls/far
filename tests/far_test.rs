use far::{Far, Faregex, Compare};

#[test]
fn when_faregex_is_empty() {
    let far = Faregex::new();
    assert_eq!(far.get_pattern (), "");
    assert_eq!(far.get_template(), "");
    
    let src = "hello";
    let dst = far.see(src);
    assert!   (dst.unchanged());
    assert_eq!(dst.len(), 1);
    assert_eq!(dst.new(), src);
    assert_eq!(dst.old(), src);
}

#[test]
fn when_faregex_is_not_empty() {
    for (mut f, n) in vec![
        (Box::new(Faregex::new()) as Box<dyn Far>, "Faregex")
    ] {
        for (i, c) in vec![(
                "", "",
                vec![
                    ["QAQ", "QAQ"]
                ]
            ), (
                r"\.\.", r".",
                vec![
                    ["..", "."],
                    ["......", "..."],
                    ["._..___....", "._.___.."],
                    ["OTZ", "OTZ"],
                ]
            ), (
                r"(\d+)\+(\d+)", r"$2+$1",
                vec![
                    ["", ""],
                    ["1+2", "2+1"],
                    ["?+1", "?+1"],
                    ["1+?", "1+?"],
                    ["1+?+3+4", "1+?+4+3"],
                    ["?+3+3+4", "?+3+3+4"],
                    ["1+1+1+1+1","1+1+1+1+1"],
                    ["1+2+3+4+5+6+7+8","2+1+4+3+6+5+8+7"],
                ]
            ), (
                r"(?P<hello>\d+)", r"",
                vec![
                    ["", ""],
                    ["1+2", "+"],
                    ["?+1", "?+"],
                    ["h1z1", "hz"],
                    ["he110w0r1d", "hewrd"],
                    ["1234567890", ""],
                ]
            ),
        ].iter().enumerate() {
            assert!(f. set_pattern(c.0).is_none(), "CASE({}, {})", n, i);
            assert!(f.set_template(c.1).is_none(), "CASE({}, {})", n, i);
            assert_eq!(f. get_pattern(), c.0, "CASE({}, {})", n, i);
            assert_eq!(f.get_template(), c.1, "CASE({}, {})", n, i);
    
            for (j, s) in c.2.iter().enumerate() {
                let o = f.see(s[0]);
                assert_eq!(o.old(), s[0], "CASE({}, {}, {})", n, i, j);
                assert_eq!(o.new(), s[1], "CASE({}, {}, {})", n, i, j);
                assert_eq!(o.unchanged(), s[0] == s[1], "CASE({}, {}, {})", n, i, j);
            }
        }
    }
}

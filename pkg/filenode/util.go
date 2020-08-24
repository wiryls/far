package filenode

import (
	"os"
	"path/filepath"
	"strings"
)

// splite filepath by `PathSeparator`. This function is different from
// `strings.Split(path, os.PathSeparator)`. As it preserves the `/`
// after volume name. e.g. Run:
//
//     Splite("C:\\H\\W")
//
// we get `["C:\\", "H", "W"]`
//
// This is because `os.Join("C:", "H", "W")` will return
// `C:H\\W` instead of our `C:\\H\\W"`.
//
// See: https://github.com/golang/go/issues/11551
func splite(path string) []string {
	const sep = string(os.PathSeparator)

	buf := []string{}
	cur := path
	vol := len(filepath.VolumeName(cur))
	if vol != 0 {
		vol = vol + 1
		buf = append(buf, cur[:vol])
		cur = path[vol:]
	}

	for cur != "" {
		i := strings.IndexRune(cur, os.PathSeparator)
		if i > 0 {
			buf = append(buf, strings.Trim(cur[:i], sep))
			cur = cur[i+1:]
		} else {
			buf = append(buf, strings.Trim(cur, sep))
			cur = cur[:0]
		}
	}

	return buf
}

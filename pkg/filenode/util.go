package filenode

import (
	"math/rand"
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
	cur := filepath.Clean(path)
	vol := len(filepath.VolumeName(cur))
	if vol != 0 {
		vol = vol + 1
		buf = append(buf, cur[:vol])
		cur = cur[vol:]
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

func isStringInSet(str string, set []string) bool {
	for i := range set {
		if str == set[i] {
			return true
		}
	}
	return false
}

func makeRandomPath(nameL, nameR, deepthL, deepthR int, set []rune) []rune {

	size := (nameR + 1) * deepthR
	buff := make([]rune, 0, size)
	name := make([]rune, nameR)

	n := deepthL
	if deepthL < deepthR {
		n += rand.Intn(deepthR - deepthL)
	}

	for i := 0; i < n; i++ {
		m := nameL
		if nameL < nameR {
			m += rand.Intn(nameR - nameL)
		}

		for j := range name[:m] {
			name[j] = set[rand.Intn(len(set))]
		}

		buff = append(buff, '\\')
		buff = append(buff, name[:m]...)
	}

	return buff
}

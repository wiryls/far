package free

import (
	"path/filepath"
	"strconv"
	"testing"

	"github.com/stretchr/testify/assert"
)

func TestSplite(t *testing.T) {
	assert := assert.New(t)

	// For now, don't add any path of other platforms. as the `os.Separator`
	// is not same in different OS. It may need a new test for for that.
	for i, src := range [][2]string{
		{``, `.`},
		{`\\\`, ``},
		{`C:\`, `C:\`},
		{`C:\Hello`, `C:\Hello`},
		{`C:\Hello\`, `C:\Hello`},
		{`C:\Hello\World`, `C:\Hello\World`},
		{`C:\Hello\World\..\..\`, `C:\`},
	} {
		num := "Case #" + strconv.Itoa(i)
		mid := splite(src[0])
		dst := filepath.Join(mid...)
		assert.Equal(src[1], dst, num)
	}
}

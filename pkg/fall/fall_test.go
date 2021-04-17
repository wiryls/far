package fall_test

import (
	"encoding/hex"
	"math/rand"
	"strings"
	"sync"
	"testing"
	"time"

	"github.com/stretchr/testify/assert"
	"github.com/wiryls/far/pkg/fall"
)

func TestFall(t *testing.T) {
	assert := assert.New(t)

	{
		m := sync.Mutex{}
		s := func() []string {
			n := rand.Int() % 100
			i := make([]string, 0, n)
			for x := 0; x < n; x++ {
				b := make([]byte, rand.Int()%50)
				rand.Read(b)
				i = append(i, hex.EncodeToString(b))
			}
			return i
		}
		u := func(i int, os []fall.Output) {
			m.Lock()
			for _, o := range os {
				assert.Len(strings.Trim(o.Differ.New(), "_"), 0)
			}
			m.Unlock()
		}

		f := fall.New(u)
		assert.NoError(f.SetPattern("[0-9a-zA-Z]"))
		assert.NoError(f.SetTemplate("_"))

		p := sync.Mutex{}
		for i := 0; i < 3; i++ {
			go func() {
				for x, n := 0, 1000; x < n; x++ {
					switch rand.Int() % 3 {
					case 0:
						p.Lock()
						f.StopWith(m.Lock)
						p.Unlock()
						time.Sleep(time.Millisecond)
						m.Unlock()
						f.Flow(s())

					case 1:
						f.Flow(s())

					case 2:
						assert.NoError(f.SetPattern("[0-9a-zA-Z]"))
					}
					time.Sleep(time.Millisecond)
				}
			}()
		}
		time.Sleep(time.Second * 5)
		f.Stop()
	}
}

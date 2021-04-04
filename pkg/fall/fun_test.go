package fall

import (
	"strconv"
	"testing"

	"github.com/stretchr/testify/assert"
)

func TestIntsUnique(t *testing.T) {
	assert := assert.New(t)

	for i, c := range [][2][]int{
		{
			{},
			{},
		},
		{
			{1, 2, 3, 4},
			{1, 2, 3, 4},
		},
		{
			{1, 1, 1, 1, 2, 3, 4},
			{1, 2, 3, 4},
		},
		{
			{1, 2, 2, 2, 2, 3, 2, 2},
			{1, 2, 3, 2},
		},
		{
			{1, 1, 2, 2, 3, 3, 4, 4},
			{1, 2, 3, 4},
		},
		{
			{0, 0, 0, 0, 0, 0, 0, 0},
			{0},
		},
	} {
		desc := "Case #" + strconv.Itoa(i)
		assert.Equal(c[1], IntsUnique(c[0]), desc)
	}
}

func TestIntsPair(t *testing.T) {
	assert := assert.New(t)

	for i, c := range []struct {
		I []int
		O [][2]int
	}{
		{
			[]int{},
			[][2]int{},
		},
		{
			[]int{1, 2, 3, 4, 5},
			[][2]int{
				{1, 5},
			},
		},
		{
			[]int{0, 1, 3, 4, 5},
			[][2]int{
				{0, 1},
				{3, 5},
			},
		},
		{
			[]int{0, 2, 4, 6},
			[][2]int{
				{0, 0},
				{2, 2},
				{4, 4},
				{6, 6},
			},
		},
		{
			[]int{0, 1, 2, 3, 5},
			[][2]int{
				{0, 3},
				{5, 5},
			},
		},
		{
			[]int{0, 2, 3, 5},
			[][2]int{
				{0, 0},
				{2, 3},
				{5, 5},
			},
		},
	} {
		desc := "Case #" + strconv.Itoa(i)
		assert.Equal(c.O, IntsPairs(c.I), desc)
	}
}

func TestRemoveItems(t *testing.T) {
	assert := assert.New(t)

	_0, _1, _2, _3, _4, _5, _6, _7 :=
		&Item{Stat: 0}, &Item{Stat: 1}, &Item{Stat: 2}, &Item{Stat: 3},
		&Item{Stat: 4}, &Item{Stat: 5}, &Item{Stat: 6}, &Item{Stat: 7}

	RemoveItemByConditionAdapter := func(items []*Item, indexes []int) []*Item {
		in := func(item *Item) bool {
			for _, k := range indexes {
				if k == int(item.Stat) {
					return true
				}
			}
			return false
		}
		return RemoveItemByCondition(items, in)
	}

	for i, f := range map[string]func([]*Item, []int) []*Item{
		"RemoveItemFromTailToHead": RemoveItemFromTailToHead,
		"RemoveItemFromHeadToTail": RemoveItemFromHeadToTail,
		"RemoveItemByIndexes":      RemoveItemByIndexes,
		"RemoveItemByCondition":    RemoveItemByConditionAdapter,
	} {
		for j, c := range []struct {
			source  []*Item
			indexes []int
			result  []*Item
		}{
			{
				nil,
				[]int{4},
				nil,
			},
			{
				[]*Item{_0, _1, _2, _3, _4, _5, _6},
				[]int{4},
				[]*Item{_0, _1, _2, _3, _5, _6},
			},
			{
				[]*Item{_0, _1, _2, _3, _4, _5, _6},
				[]int{6},
				[]*Item{_0, _1, _2, _3, _4, _5},
			},
			{
				[]*Item{_0, _1, _2, _3, _4, _5, _6, _7},
				[]int{0},
				[]*Item{_1, _2, _3, _4, _5, _6, _7},
			},
			{
				[]*Item{_0, _1, _2, _3, _4, _5, _6, _7},
				[]int{0, 1, 2, 3, 4, 5, 6, 7},
				[]*Item{},
			},
			{
				[]*Item{_0, _1, _2, _3, _4, _5, _6},
				[]int{0, 1, 2},
				[]*Item{_3, _4, _5, _6},
			},
			{
				[]*Item{_0, _1, _2, _3, _4, _5, _6},
				[]int{4, 5, 6},
				[]*Item{_0, _1, _2, _3},
			},
			{
				[]*Item{_0, _1, _2, _3, _4, _5, _6, _7},
				[]int{0, 1, 3, 4, 6},
				[]*Item{_2, _5, _7},
			},
			{
				[]*Item{_0, _1, _2, _3, _4, _5, _6},
				[]int{1, 2, 3, 4, 5},
				[]*Item{_0, _6},
			},
			{
				[]*Item{_0, _1, _2, _3, _4, _5, _6},
				[]int{1, 3, 5},
				[]*Item{_0, _2, _4, _6},
			},
		} {
			desc := "Case " + i + "#" + strconv.Itoa(j)
			assert.Equal(c.result, f(c.source, c.indexes), desc)
		}
	}
}

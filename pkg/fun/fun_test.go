package fun_test

import (
	"strconv"
	"testing"

	"github.com/stretchr/testify/assert"
	"github.com/wiryls/far/pkg/fun"
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
		assert.Equal(c[1], fun.IntsUnique(c[0]), desc)
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
		assert.Equal(c.O, fun.IntsPairs(c.I), desc)
	}
}

func TestRemoveItems(t *testing.T) {
	assert := assert.New(t)

	_0, _1, _2, _3, _4, _5, _6, _7 :=
		0, 1, 2, 3, 4, 5, 6, 7

	RemoveItemByConditionAdapter := func(items []interface{}, indexes []int) []interface{} {
		in := func(item interface{}) bool {
			for _, k := range indexes {
				if k == item.(int) {
					return true
				}
			}
			return false
		}
		return fun.RemoveItemByCondition(items, in)
	}

	for i, f := range map[string]func([]interface{}, []int) []interface{}{
		"RemoveItemFromTailToHead": fun.RemoveItemFromTailToHead,
		"RemoveItemFromHeadToTail": fun.RemoveItemFromHeadToTail,
		"RemoveItemByIndexes":      fun.RemoveItemByIndexes,
		"RemoveItemByCondition":    RemoveItemByConditionAdapter,
	} {
		for j, c := range []struct {
			source  []interface{}
			indexes []int
			result  []interface{}
		}{
			{
				nil,
				[]int{4},
				nil,
			},
			{
				[]interface{}{_0, _1, _2, _3, _4, _5, _6},
				[]int{4},
				[]interface{}{_0, _1, _2, _3, _5, _6},
			},
			{
				[]interface{}{_0, _1, _2, _3, _4, _5, _6},
				[]int{6},
				[]interface{}{_0, _1, _2, _3, _4, _5},
			},
			{
				[]interface{}{_0, _1, _2, _3, _4, _5, _6, _7},
				[]int{0},
				[]interface{}{_1, _2, _3, _4, _5, _6, _7},
			},
			{
				[]interface{}{_0, _1, _2, _3, _4, _5, _6, _7},
				[]int{0, 1, 2, 3, 4, 5, 6, 7},
				[]interface{}{},
			},
			{
				[]interface{}{_0, _1, _2, _3, _4, _5, _6},
				[]int{0, 1, 2},
				[]interface{}{_3, _4, _5, _6},
			},
			{
				[]interface{}{_0, _1, _2, _3, _4, _5, _6},
				[]int{4, 5, 6},
				[]interface{}{_0, _1, _2, _3},
			},
			{
				[]interface{}{_0, _1, _2, _3, _4, _5, _6, _7},
				[]int{0, 1, 3, 4, 6},
				[]interface{}{_2, _5, _7},
			},
			{
				[]interface{}{_0, _1, _2, _3, _4, _5, _6},
				[]int{1, 2, 3, 4, 5},
				[]interface{}{_0, _6},
			},
			{
				[]interface{}{_0, _1, _2, _3, _4, _5, _6},
				[]int{1, 3, 5},
				[]interface{}{_0, _2, _4, _6},
			},
		} {
			desc := "Case " + i + "#" + strconv.Itoa(j)
			assert.Equal(c.result, f(c.source, c.indexes), desc)
		}
	}
}

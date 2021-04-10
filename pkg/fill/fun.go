package fill

import "sort"

// Limited number to limit.
func Limited(limit int) func(int) int {
	return func(size int) int {
		if size > limit {
			return limit
		}
		return size
	}
}

// IntsUnique just deduplicates adjacent numbers. It's similer to the STL
// version.
func IntsUnique(nums []int) []int {
	k, j := 0, 0
	for i := range nums {
		if nums[j] != nums[i] {
			j = i
		}
		if j == i {
			nums[k] = nums[i]
			k++
		}
	}
	return nums[:k]
}

func IntsPairs(nums []int) [][2]int {
	pairs := [][2]int{}
	for _, k := range nums {
		n := len(pairs)
		if n == 0 {
			pairs = append(pairs, [2]int{k, k})
		} else if pairs[n-1][1]+1 == k {
			pairs[n-1][1]++
		} else {
			pairs = append(pairs, [2]int{k, k})
		}
	}
	return pairs
}

// RemoveItemByCondition remove any item if cond(item) is true.
func RemoveItemByCondition(items []interface{}, cond func(interface{}) bool) []interface{} {

	k, n := 0, len(items)
	for i := 0; i < n; i++ {
		if i != k {
			items[k] = items[i]
		}
		if !cond(items[i]) {
			k++
		}
	}

	return items[:k]
}

// RemoveItemByIndexes
func RemoveItemByIndexes(items []interface{}, indexes []int) []interface{} {
	if len(items) == 0 || len(indexes) == 0 {
		return items
	}

	if !sort.IntsAreSorted(indexes) {
		sort.Ints(indexes)
	}

	indexes = IntsUnique(indexes)
	for 0 < len(indexes) && indexes[0] < 0 {
		indexes = indexes[1:]
	}
	for 0 < len(indexes) && indexes[len(indexes)-1] >= len(items) {
		indexes = indexes[:len(indexes)-1]
	}

	if len(indexes) == 0 {
		return items
	}

	if min, max := indexes[0], indexes[len(indexes)-1]; min+max < len(items) {
		items = RemoveItemFromTailToHead(items, indexes)
	} else {
		items = RemoveItemFromHeadToTail(items, indexes)
	}

	return items
}

// RemoveItemFromTailToHead will remove `items` by `indexes` from tail to
// head.
//
// e.g. Delete [1, 2] from [0, 1, 2, 3] will become [0, 1, 0, 3] then [0, 3].
//
//  - `indexes` must be sorted in ascending order
//  - `indexes` must be in range `[0, len(items))`.
//  - there should not be any duplicate `indexes`.
func RemoveItemFromTailToHead(items []interface{}, indexes []int) []interface{} {
	if len(items) == 0 || len(indexes) == 0 {
		return items
	}

	i := len(indexes) - 1
	x, y := indexes[i], indexes[i]

	for x >= 0 && i >= 0 {
		if x == indexes[i] {
			i--
		} else {
			items[y] = items[x]
			y--
		}
		x--
	}

	for x >= 0 {
		items[y] = items[x]
		x--
		y--
	}

	return items[y+1:]
}

// RemoveItemFromHeadToTail is similar to `RemoveItemFromTailToHead` except
// the removing order is from head to tail.
//
// e.g. Delete [1, 2] from [0, 1, 2, 3] will become [0, 3, 2, 3] then [0, 3].
//
//  - `indexes` must be sorted in ascending order
//  - `indexes` must be in range `[0, len(items))`.
//  - there should not be any duplicate `indexes`.
func RemoveItemFromHeadToTail(items []interface{}, indexes []int) []interface{} {
	if len(items) == 0 || len(indexes) == 0 {
		return items
	}

	x, y, n := indexes[0], indexes[0], len(items)

	for x < n && len(indexes) != 0 {
		if x == indexes[0] {
			indexes = indexes[1:]
		} else {
			items[y] = items[x]
			y++
		}
		x++
	}

	for x < n {
		items[y] = items[x]
		y++
		x++
	}

	return items[:y]
}

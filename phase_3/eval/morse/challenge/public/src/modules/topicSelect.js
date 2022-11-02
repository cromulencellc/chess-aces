'use strict';


define('topicSelect', ['components'], function (components) {
	const TopicSelect = {};
	let lastSelected;

	let topicsContainer;

	TopicSelect.init = function (onSelect) {
		topicsContainer = $('[component="category"]');
		topicsContainer.on('selectstart', '[component="topic/select"]', function (ev) {
			ev.preventDefault();
		});

		topicsContainer.on('click', '[component="topic/select"]', function (ev) {
			const select = $(this);

			if (ev.shiftKey) {
				selectRange($(this).parents('[component="category/topic"]').attr('data-tid'));
				lastSelected = select;
				return false;
			}

			const isSelected = select.parents('[data-tid]').hasClass('selected');
			toggleSelect(select, !isSelected);
			lastSelected = select;
			if (typeof onSelect === 'function') {
				onSelect();
			}
		});
	};

	function toggleSelect(select, isSelected) {
		select.toggleClass('fa-check-square-o', isSelected);
		select.toggleClass('fa-square-o', !isSelected);
		select.parents('[component="category/topic"]').toggleClass('selected', isSelected);
	}

	TopicSelect.getSelectedTids = function () {
		const tids = [];
		if (!topicsContainer) {
			return tids;
		}
		topicsContainer.find('[component="category/topic"].selected').each(function () {
			tids.push($(this).attr('data-tid'));
		});
		return tids;
	};

	TopicSelect.unselectAll = function () {
		if (topicsContainer) {
			topicsContainer.find('[component="category/topic"].selected').removeClass('selected');
			topicsContainer.find('[component="topic/select"]').toggleClass('fa-check-square-o', false).toggleClass('fa-square-o', true);
		}
	};

	function selectRange(clickedTid) {
		if (!lastSelected) {
			lastSelected = $('[component="category/topic"]').first().find('[component="topic/select"]');
		}

		const isClickedSelected = components.get('category/topic', 'tid', clickedTid).hasClass('selected');

		const clickedIndex = getIndex(clickedTid);
		const lastIndex = getIndex(lastSelected.parents('[component="category/topic"]').attr('data-tid'));
		selectIndexRange(clickedIndex, lastIndex, !isClickedSelected);
	}

	function selectIndexRange(start, end, isSelected) {
		if (start > end) {
			const tmp = start;
			start = end;
			end = tmp;
		}

		for (let i = start; i <= end; i += 1) {
			const topic = $('[component="category/topic"]').eq(i);
			toggleSelect(topic.find('[component="topic/select"]'), isSelected);
		}
	}

	function getIndex(tid) {
		return components.get('category/topic', 'tid', tid).index('[component="category/topic"]');
	}

	return TopicSelect;
});

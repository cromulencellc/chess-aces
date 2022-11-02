'use strict';

const user = require('../../user');
const meta = require('../../meta');
const helpers = require('../helpers');
const groups = require('../../groups');
const accountHelpers = require('./helpers');
const privileges = require('../../privileges');
const file = require('../../file');

const editController = module.exports;

editController.get = async function (req, res, next) {
	const [userData, canUseSignature] = await Promise.all([
		accountHelpers.getUserDataByUserSlug(req.params.userslug, req.uid, req.query),
		privileges.global.can('signature', req.uid),
	]);
	if (!userData) {
		return next();
	}
	userData.maximumSignatureLength = meta.config.maximumSignatureLength;
	userData.maximumAboutMeLength = meta.config.maximumAboutMeLength;
	userData.maximumProfileImageSize = meta.config.maximumProfileImageSize;
	userData.allowProfilePicture = !userData.isSelf || !!meta.config['reputation:disabled'] || userData.reputation >= meta.config['min:rep:profile-picture'];
	userData.allowCoverPicture = !userData.isSelf || !!meta.config['reputation:disabled'] || userData.reputation >= meta.config['min:rep:cover-picture'];
	userData.allowProfileImageUploads = meta.config.allowProfileImageUploads;
	userData.allowedProfileImageExtensions = user.getAllowedProfileImageExtensions().map(ext => `.${ext}`).join(', ');
	userData.allowMultipleBadges = meta.config.allowMultipleBadges === 1;
	userData.allowAccountDelete = meta.config.allowAccountDelete === 1;
	userData.allowWebsite = !userData.isSelf || !!meta.config['reputation:disabled'] || userData.reputation >= meta.config['min:rep:website'];
	userData.allowAboutMe = !userData.isSelf || !!meta.config['reputation:disabled'] || userData.reputation >= meta.config['min:rep:aboutme'];
	userData.allowSignature = canUseSignature && (!userData.isSelf || !!meta.config['reputation:disabled'] || userData.reputation >= meta.config['min:rep:signature']);
	userData.profileImageDimension = meta.config.profileImageDimension;
	userData.defaultAvatar = user.getDefaultAvatar();

	userData.groups = userData.groups.filter(g => g && g.userTitleEnabled && !groups.isPrivilegeGroup(g.name) && g.name !== 'registered-users');

	if (!userData.allowMultipleBadges) {
		userData.groupTitle = userData.groupTitleArray[0];
	}

	userData.groups.sort((a, b) => {
		const i1 = userData.groupTitleArray.indexOf(a.name);
		const i2 = userData.groupTitleArray.indexOf(b.name);
		if (i1 === -1) {
			return 1;
		} else if (i2 === -1) {
			return -1;
		}
		return i1 - i2;
	});
	userData.groups.forEach((group) => {
		group.userTitle = group.userTitle || group.displayName;
		group.selected = userData.groupTitleArray.includes(group.name);
	});
	userData.groupSelectSize = Math.min(10, Math.max(5, userData.groups.length + 1));

	userData.title = `[[pages:account/edit, ${userData.username}]]`;
	userData.breadcrumbs = helpers.buildBreadcrumbs([
		{
			text: userData.username,
			url: `/user/${userData.userslug}`,
		},
		{
			text: '[[user:edit]]',
		},
	]);
	userData.editButtons = [];
	res.render('account/edit', userData);
};

editController.password = async function (req, res, next) {
	await renderRoute('password', req, res, next);
};

editController.username = async function (req, res, next) {
	await renderRoute('username', req, res, next);
};

editController.email = async function (req, res, next) {
	const targetUid = await user.getUidByUserslug(req.params.userslug);
	if (!targetUid) {
		return next();
	}

	const [isAdminOrGlobalMod, canEdit] = await Promise.all([
		user.isAdminOrGlobalMod(req.uid),
		privileges.users.canEdit(req.uid, targetUid),
	]);

	if (!isAdminOrGlobalMod && !canEdit) {
		return next();
	}

	req.session.returnTo = `/uid/${targetUid}`;
	req.session.registration = req.session.registration || {};
	req.session.registration.updateEmail = true;
	req.session.registration.uid = targetUid;
	helpers.redirect(res, '/register/complete');
};

async function renderRoute(name, req, res, next) {
	const userData = await getUserData(req, next);
	if (!userData) {
		return next();
	}
	if (meta.config[`${name}:disableEdit`] && !userData.isAdmin) {
		return helpers.notAllowed(req, res);
	}

	if (name === 'password') {
		userData.minimumPasswordLength = meta.config.minimumPasswordLength;
		userData.minimumPasswordStrength = meta.config.minimumPasswordStrength;
	}

	userData.title = `[[pages:account/edit/${name}, ${userData.username}]]`;
	userData.breadcrumbs = helpers.buildBreadcrumbs([
		{
			text: userData.username,
			url: `/user/${userData.userslug}`,
		},
		{
			text: '[[user:edit]]',
			url: `/user/${userData.userslug}/edit`,
		},
		{
			text: `[[user:${name}]]`,
		},
	]);

	res.render(`account/edit/${name}`, userData);
}

async function getUserData(req) {
	const userData = await accountHelpers.getUserDataByUserSlug(req.params.userslug, req.uid, req.query);
	if (!userData) {
		return null;
	}

	userData.hasPassword = await user.hasPassword(userData.uid);
	return userData;
}

editController.uploadPicture = async function (req, res, next) {
	const userPhoto = req.files.files[0];
	try {
		const updateUid = await user.getUidByUserslug(req.params.userslug);
		const isAllowed = await privileges.users.canEdit(req.uid, updateUid);
		if (!isAllowed) {
			return helpers.notAllowed(req, res);
		}
		await user.checkMinReputation(req.uid, updateUid, 'min:rep:profile-picture');

		const image = await user.uploadCroppedPictureFile({
			callerUid: req.uid,
			uid: updateUid,
			file: userPhoto,
		});

		res.json([{
			name: userPhoto.name,
			url: image.url,
		}]);
	} catch (err) {
		next(err);
	} finally {
		await file.delete(userPhoto.path);
	}
};

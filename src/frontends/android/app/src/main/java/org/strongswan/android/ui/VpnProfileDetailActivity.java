/*
 * Copyright (C) 2012-2016 Tobias Brunner
 * Copyright (C) 2012 Giuliano Grassi
 * Copyright (C) 2012 Ralf Sager
 * HSR Hochschule fuer Technik Rapperswil
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

package org.strongswan.android.ui;

import android.content.Intent;
import android.os.Bundle;
import android.support.v4.content.LocalBroadcastManager;
import android.support.v7.app.AppCompatActivity;
import android.text.Editable;
import android.text.SpannableString;
import android.text.Spanned;
import android.text.TextUtils;
import android.text.TextWatcher;
import android.util.Log;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.widget.ArrayAdapter;
import android.widget.EditText;
import android.widget.MultiAutoCompleteTextView;

import net.postquantum.vpn.R;
import org.strongswan.android.data.VpnProfile;
import org.strongswan.android.data.VpnProfileDataSource;
import org.strongswan.android.ui.widget.TextInputLayoutHelper;
import org.strongswan.android.utils.Constants;

import java.util.UUID;

public class VpnProfileDetailActivity extends AppCompatActivity
{
	private VpnProfileDataSource mDataSource;
	private Long mId;
	private VpnProfile mProfile;
	private MultiAutoCompleteTextView mName;
	private TextInputLayoutHelper mNameWrap;
	private EditText mGateway;
	private TextInputLayoutHelper mGatewayWrap;
	private EditText mPsk;

	@Override
	public void onCreate(Bundle savedInstanceState)
	{
		super.onCreate(savedInstanceState);

		/* the title is set when we load the profile, if any */
		getSupportActionBar().setDisplayHomeAsUpEnabled(true);

		mDataSource = new VpnProfileDataSource(this);
		mDataSource.open();

		setContentView(R.layout.profile_detail_view);

		mName = (MultiAutoCompleteTextView)findViewById(R.id.name);
		mNameWrap = (TextInputLayoutHelper)findViewById(R.id.name_wrap);
		mGateway = (EditText)findViewById(R.id.gateway);
		mGatewayWrap = (TextInputLayoutHelper) findViewById(R.id.gateway_wrap);
		mPsk = (EditText)findViewById(R.id.psk);


		final SpaceTokenizer spaceTokenizer = new SpaceTokenizer();
		mName.setTokenizer(spaceTokenizer);
		//mRemoteId.setTokenizer(spaceTokenizer);
		final ArrayAdapter<String> completeAdapter = new ArrayAdapter<>(this, android.R.layout.simple_dropdown_item_1line);
		mName.setAdapter(completeAdapter);
		//mRemoteId.setAdapter(completeAdapter);

		mGateway.addTextChangedListener(new TextWatcher() {
			@Override
			public void beforeTextChanged(CharSequence s, int start, int count, int after) {}

			@Override
			public void onTextChanged(CharSequence s, int start, int before, int count) {}

			@Override
			public void afterTextChanged(Editable s)
			{
				completeAdapter.clear();
				completeAdapter.add(mGateway.getText().toString());
				if (TextUtils.isEmpty(mGateway.getText()))
				{
					mNameWrap.setHelperText(getString(R.string.profile_name_hint));
					//mRemoteIdWrap.setHelperText(getString(R.string.profile_remote_id_hint));
				}
				else
				{
					mNameWrap.setHelperText(String.format(getString(R.string.profile_name_hint_gateway), mGateway.getText()));
					//mRemoteIdWrap.setHelperText(String.format(getString(R.string.profile_remote_id_hint_gateway), mGateway.getText()));
				}
			}
		});

		mId = savedInstanceState == null ? null : savedInstanceState.getLong(VpnProfileDataSource.KEY_ID);
		if (mId == null)
		{
			Bundle extras = getIntent().getExtras();
			mId = extras == null ? null : extras.getLong(VpnProfileDataSource.KEY_ID);
		}

		loadProfileData(savedInstanceState);

	}

	@Override
	protected void onDestroy()
	{
		super.onDestroy();
		mDataSource.close();
	}

	@Override
	protected void onSaveInstanceState(Bundle outState)
	{
		super.onSaveInstanceState(outState);
		if (mId != null)
		{
			outState.putLong(VpnProfileDataSource.KEY_ID, mId);
		}
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu)
	{
		MenuInflater inflater = getMenuInflater();
		inflater.inflate(R.menu.profile_edit, menu);
		return true;
	}

	@Override
	public boolean onOptionsItemSelected(MenuItem item)
	{
		switch (item.getItemId())
		{
			case android.R.id.home:
			case R.id.menu_cancel:
				finish();
				return true;
			case R.id.menu_accept:
				saveProfile();
				return true;
			default:
				return super.onOptionsItemSelected(item);
		}
	}

	@Override
	protected void onActivityResult(int requestCode, int resultCode, Intent data)
	{
		switch (requestCode)
		{
			default:
				super.onActivityResult(requestCode, resultCode, data);
		}
	}


	/**
	 * Save or update the profile depending on whether we actually have a
	 * profile object or not (this was created in updateProfileData)
	 */
	private void saveProfile()
	{
		if (verifyInput())
		{
			if (mProfile != null)
			{
				updateProfileData();
				if (mProfile.getUUID() == null)
				{
					mProfile.setUUID(UUID.randomUUID());
				}
				mDataSource.updateVpnProfile(mProfile);
			}
			else
			{
				mProfile = new VpnProfile();
				updateProfileData();
				mDataSource.insertProfile(mProfile);
			}
			Intent intent = new Intent(Constants.VPN_PROFILES_CHANGED);
			intent.putExtra(Constants.VPN_PROFILES_SINGLE, mProfile.getId());
			LocalBroadcastManager.getInstance(this).sendBroadcast(intent);

			setResult(RESULT_OK, new Intent().putExtra(VpnProfileDataSource.KEY_ID, mProfile.getId()));
			finish();
		}
	}

	/**
	 * Verify the user input and display error messages.
	 * @return true if the input is valid
	 */
	private boolean verifyInput()
	{
		boolean valid = true;
		if (mGateway.getText().toString().trim().isEmpty())
		{
			mGatewayWrap.setError(getString(R.string.alert_text_no_input_gateway));
			valid = false;
		}
		return valid;
	}

	/**
	 * Update the profile object with the data entered by the user
	 */
	private void updateProfileData()
	{
		/* the name is optional, we default to the gateway if none is given */
		String name = mName.getText().toString().trim();
		String gateway = mGateway.getText().toString().trim();
		mProfile.setName(name.isEmpty() ? gateway : name);
		mProfile.setGateway(gateway);
		String psk = mPsk.getText().toString().trim();
		psk = psk.isEmpty() ? null : psk;
		mProfile.setPassword(psk);
		mProfile.setSplitTunneling(null);
	}

	/**
	 * Load an existing profile if we got an ID
	 *
	 * @param savedInstanceState previously saved state
	 */
	private void loadProfileData(Bundle savedInstanceState)
	{
		String useralias = null, local_id = null, alias = null;

		getSupportActionBar().setTitle(R.string.add_profile);
		if (mId != null && mId != 0)
		{
			mProfile = mDataSource.getVpnProfile(mId);
			if (mProfile != null)
			{
				mName.setText(mProfile.getName());
				mGateway.setText(mProfile.getGateway());
				mPsk.setText(mProfile.getPassword());
				local_id = mProfile.getLocalId();
				getSupportActionBar().setTitle(mProfile.getName());
			}
			else
			{
				Log.e(VpnProfileDetailActivity.class.getSimpleName(),
					  "VPN profile with id " + mId + " not found");
				finish();
			}
		}

	}



	/**
	 * Tokenizer implementation that separates by white-space
	 */
	public static class SpaceTokenizer implements MultiAutoCompleteTextView.Tokenizer
	{
		@Override
		public int findTokenStart(CharSequence text, int cursor)
		{
			int i = cursor;

			while (i > 0 && !Character.isWhitespace(text.charAt(i - 1)))
			{
				i--;
			}
			return i;
		}

		@Override
		public int findTokenEnd(CharSequence text, int cursor)
		{
			int i = cursor;
			int len = text.length();

			while (i < len)
			{
				if (Character.isWhitespace(text.charAt(i)))
				{
					return i;
				}
				else
				{
					i++;
				}
			}
			return len;
		}

		@Override
		public CharSequence terminateToken(CharSequence text)
		{
			int i = text.length();

			if (i > 0 && Character.isWhitespace(text.charAt(i - 1)))
			{
				return text;
			}
			else
			{
				if (text instanceof Spanned)
				{
					SpannableString sp = new SpannableString(text + " ");
					TextUtils.copySpansFrom((Spanned) text, 0, text.length(), Object.class, sp, 0);
					return sp;
				}
				else
				{
					return text + " ";
				}
			}
		}
	}
}

# Lambda Mail

Hello guys, This README serves as a temporary guide for the temporary mail API. I recommend downloading [Bruno](https://www.usebruno.com/), a collaborative tool like Postman tbut better. If you download Bruno and open the `bruno` folder in this repository, you’ll be able to view all the requests I’ve created for this API.

The official documentation for using the API is available [here](https://docs.mail.tm/). However, you can follow along with the instructions below for a practical example in Bruno.

## Getting Started

### 1. Retrieving Available Domains

The API allows email creation only with specific domains. To retrieve a list of available domains, you can make a `GET` request to `https://api.mail.tm/domains` without any parameters or body. This will return a response similar to the example below:

```json
{
  "@context": "/contexts/Domain",
  "@id": "/domains",
  "@type": "hydra:Collection",
  "hydra:totalItems": 1,
  "hydra:member": [
    {
      "@id": "/domains/67178290fd0d15bb39041edc",
      "@type": "Domain",
      "id": "67178290fd0d15bb39041edc",
      "domain": "livinitlarge.net", //This is the email domain we will use
      "isActive": true,
      "isPrivate": false,
      "createdAt": "2024-10-22T00:00:00+00:00",
      "updatedAt": "2024-10-22T00:00:00+00:00"
    }
  ]
}
```

### 2. Creating an Account
Now that you have a domain you need to create an account, which is done by making a `POST` request to `https://api.mail.tm/accounts` with a body containing the email you want from the domain you got, and a password like this:

```json
{
  "address": "zaidsaheb@livinitlarge.net",
  "password": "strongpassword!"
}
```

Which will return a response like this with a `201` status code:

```json
{
  "@context": "/contexts/Account",
  "@id": "/accounts/671fed8fc747b11c3405311b",
  "@type": "Account",
  "id": "671fed8fc747b11c3405311b",
  "address": "zaidsaheb@livinitlarge.net",
  "quota": 40000000,
  "used": 0,
  "isDisabled": false,
  "isDeleted": false,
  "createdAt": "2024-10-28T20:01:19+00:00",
  "updatedAt": "2024-10-28T20:01:19+00:00"
}
```

### 3. Send an email
You can now send an email to that address, I sent an email to `zaidsaheb@livinitlarge.net` from my personal email

### 4. Get a token
You will need a unique token to be able to access the email's inbox, which is obtained by making a `POST` request to `https://api.mail.tm/token` with the body of your new email and password like so:

```json
{
  "address": "zaidsaheb@livinitlarge.net",
  "password": "strongpassword!"
}
```
Which returns this:
```json
{
  "token": "eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzUxMiJ9.eyJpYXQiOjE3MzAxNDU3OTMsInJvbGVzIjpbIlJPTEVfVVNFUiJdLCJhZGRyZXNzIjoiemFpZHNhaGViQGxpdmluaXRsYXJnZS5uZXQiLCJpZCI6IjY3MWZlZDhmYzc0N2IxMWMzNDA1MzExYiIsIm1lcmN1cmUiOnsic3Vic2NyaWJlIjpbIi9hY2NvdW50cy82NzFmZWQ4ZmM3NDdiMTFjMzQwNTMxMWIiXX19.KY78YXWIzX0BZI0u6P5jMFJM1hyLkDNstmh9ZkkFV-3aJo-MWdOBEZBXfRP5_rvuLs6H6JcTAPSWcBu4DTVSZA",
  "@id": "/accounts/671fed8fc747b11c3405311b",
  "id": "671fed8fc747b11c3405311b"
}
```

### 5. Get inbox
Now that you have a token you can make a `GET` request to `https://api.mail.tm/messages?page=1` with the header set as `"Authorization": "Bearer TOKEN"` like this:

```json
//Check Bruno for a better representation
"Authorization": "Bearer eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzUxMiJ9.eyJpYXQiOjE3MzAxNDU3OTMsInJvbGVzIjpbIlJPTEVfVVNFUiJdLCJhZGRyZXNzIjoiemFpZHNhaGViQGxpdmluaXRsYXJnZS5uZXQiLCJpZCI6IjY3MWZlZDhmYzc0N2IxMWMzNDA1MzExYiIsIm1lcmN1cmUiOnsic3Vic2NyaWJlIjpbIi9hY2NvdW50cy82NzFmZWQ4ZmM3NDdiMTFjMzQwNTMxMWIiXX19.KY78YXWIzX0BZI0u6P5jMFJM1hyLkDNstmh9ZkkFV-3aJo-MWdOBEZBXfRP5_rvuLs6H6JcTAPSWcBu4DTVSZA"
```

Which returns the inbox!

```json
{
  "@context": "/contexts/Message",
  "@id": "/messages",
  "@type": "hydra:Collection",
  "hydra:totalItems": 1,
  "hydra:member": [
    {
      "@id": "/messages/671feeeb08f0647ffd1cf7af",
      "@type": "Message",
      "id": "671feeeb08f0647ffd1cf7af",
      "msgid": "<CANbOaXNn3Y3Z-0zk5hMfkTEWQEhNKV6ySvQ6gJ9VWnHLw2CSUQ@mail.gmail.com>",
      "from": {
        "address": "zaidsaheb4@gmail.com",
        "name": "Zaid Saheb"
      },
      "to": [
        {
          "address": "zaidsaheb@livinitlarge.net",
          "name": ""
        }
      ],
      "subject": "Hello Subject",
      "intro": "Hello World! Sincerely, Zaid",
      "seen": false,
      "isDeleted": false,
      "hasAttachments": false,
      "size": 3079,
      "downloadUrl": "/messages/671feeeb08f0647ffd1cf7af/download",
      "sourceUrl": "/sources/671feeeb08f0647ffd1cf7af",
      "createdAt": "2024-10-28T20:06:50+00:00",
      "updatedAt": "2024-10-28T20:07:07+00:00",
      "accountId": "/accounts/671fed8fc747b11c3405311b"
    }
  ]
}
```


 

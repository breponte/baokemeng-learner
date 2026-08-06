# Translation

## Setup

### Google Cloud Project Setup

In order to utilize the Google Cloud Translation API, a Google Cloud project must be setup. Official instructions are found [here](https://docs.cloud.google.com/translate/docs/setup). The `Project ID` is needed as an environment variable to the Docker Compose file.

Google Cloud will outline the steps in setting up the project, from creation, to billing, to enabling the API. After enabling the API, the next section in this README.md outlines this project's specific authentication instructions for local development environment.

### Google Cloud Authenticate with REST

#### Local Development Environment

First, set up Google Cloud CLI locally according to these [instructions](https://docs.cloud.google.com/docs/authentication/set-up-adc-local-dev-environment#local-user-cred).

[Install](https://docs.cloud.google.com/sdk/docs/install-sdk) the Google Cloud CLI.

For a local shell, create local authentication credentials for your user account via `gcloud auth application-default login`

#### Application Default Credentials

This method is the preferred option for authenticating a REST call in a production environment, because ADC finds credentials from the resource where your code is running (such as a Compute Engine virtual machine). You can also use ADC to authenticate in a local development environment. Official instructions are found [here](https://docs.cloud.google.com/docs/authentication/rest#rest-request).

In order to retrieve the access token for your Google Cloud project, run `gcloud auth application-default print-access-token` in a local development environment after Google Cloud CLI is installed. This is needed as an environment variable to the Docker Compose file.

Verify that the local development environment has access to the Google Cloud project by running the following command with your own `PROJECT_ID`:

```
curl -X GET \
     -H "Authorization: Bearer $(gcloud auth application-default print-access-token)" \
     "https://cloudresourcemanager.googleapis.com/v3/projects/PROJECT_ID"
```

Future REST API requests will follow this format.

## TODO

- None

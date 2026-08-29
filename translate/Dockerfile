FROM golang:1.26.5 AS builder

WORKDIR /app
COPY go.mod .
COPY translate/ ./translate/
RUN go build -o /translate ./translate/

FROM gcr.io/google.com/cloudsdktool/google-cloud-cli

COPY --from=builder /translate /usr/local/bin/translate

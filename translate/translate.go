package main

import (
	"bufio"
	"bytes"
	"encoding/json"
	"fmt"
	"net/http"
	"os"
	"strings"
)

type Post struct {
	Q	string	`json:"q"`
	Source	string	`json:"source"`
	Target	string	`json:"target"`
	Format	string	`json:"format"`
}

type TranslationResponse struct {
	Data TranslationData `json:"data"`
}

type TranslationData struct {
	Translations []Translation `json:"translations"`
}

type Translation struct {
	TranslatedText string `json:"translatedText"`
}

func main() {
	accessToken := os.Getenv("GCP_ACCESS_TOKEN")
	projectId := os.Getenv("GCP_PROJECT_ID")
	if projectId == "" || accessToken == "" {
		panic("Failed to load environment variables")
	}

	// HTTP endpoint
	posturl := "https://translation.googleapis.com/language/translate/v2"

	// Read text from STDIN
	fmt.Fprint(os.Stderr, "Enter text to translate: ")
	scanner := bufio.NewScanner(os.Stdin)
	scanner.Scan()
	inputText := strings.TrimSpace(scanner.Text())

	// JSON body
	post := Post{
		Q:      inputText,
		Source: "en",
		Target: "zh",
		Format: "text",
	}
	body, err := json.Marshal(post)
	if err != nil {
		panic(err)
	}

	// Create a HTTP post request
	r, err := http.NewRequest("POST", posturl, bytes.NewBuffer(body))
	if err != nil {
		panic(err)
	}

	auth := fmt.Sprintf("Bearer %s", accessToken)
	r.Header.Add("Authorization", auth)
	r.Header.Add("x-goog-user-project", projectId)
	r.Header.Add("Content-Type", "application/json; charset=utf-8")

	client := &http.Client{}
	res, err := client.Do(r)
	if err != nil {
		panic(err)
	}

	defer res.Body.Close()

	response := TranslationResponse{}
	derr := json.NewDecoder(res.Body).Decode(&response)
	if derr != nil {
		panic(derr)
	}

	if res.StatusCode != http.StatusOK {
		panic(res.Status)
	}

	fmt.Println("Translation:", response.Data.Translations[0].TranslatedText)
}

# Callbacks

Callbacks verwenden `CALL`, liefern `Result` und erhalten gegebenenfalls undurchsichtiges `user_data`. Sie dürfen keine Exceptions über die ABI-Grenze werfen, geliehene Daten über deren Lebensdauer hinaus behalten oder als nicht reentrant dokumentierte Operationen rekursiv aufrufen.

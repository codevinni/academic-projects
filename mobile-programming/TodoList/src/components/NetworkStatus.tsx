import { useState, useEffect } from "react";

function NetworkStatus() {
  const [isOnline, setIsOnline] = useState<boolean>(() => {
    return typeof navigator !== "undefined" && typeof navigator.onLine === "boolean"
      ? navigator.onLine
      : true;
  });

  useEffect(() => {
    const handleOnline = () => setIsOnline(true);
    const handleOffline = () => setIsOnline(false);

    window.addEventListener("online", handleOnline);
    window.addEventListener("offline", handleOffline);

    // Sincroniza dinamicamente ao retomar o foco da aba ou visibilidade do PWA no celular
    const syncStatus = () => {
      if (typeof navigator !== "undefined" && typeof navigator.onLine === "boolean") {
        setIsOnline(navigator.onLine);
      }
    };

    window.addEventListener("focus", syncStatus);
    document.addEventListener("visibilitychange", syncStatus);

    return () => {
      window.removeEventListener("online", handleOnline);
      window.removeEventListener("offline", handleOffline);
      window.removeEventListener("focus", syncStatus);
      document.removeEventListener("visibilitychange", syncStatus);
    };
  }, []);

  return (
    <div
      className={`network-status-badge ${isOnline ? "network-status--online" : "network-status--offline"}`}
      role="status"
      aria-live="polite"
      aria-label={isOnline ? "Status de conexão: Online" : "Status de conexão: Offline"}
      title={isOnline ? "Você está conectado à internet" : "Você está offline (sem conexão à internet)"}
    >
      <span className="network-status-dot" aria-hidden="true" />
      <span className="network-status-text">
        {isOnline ? "Online" : "Offline"}
      </span>
    </div>
  );
}

export default NetworkStatus;

